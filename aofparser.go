package aof

import (
	"bufio"
	"errors"
	"io"
	"os"
	"strconv"
)

var kErrorAofWithRdbPremptNotSupported = errors.New("ErrorAofWithRdbPremptNotSupported")

type Parser struct {
	aofPath string
	file    *os.File
	reader  *bufio.Reader
	eof     bool
}

func NewParser(aofPath string) (*Parser, error) {
	file, err := os.Open(aofPath)
	if err != nil {
		return nil, err
	}

	parser := &Parser{
		aofPath: aofPath,
		file:    file,
		reader:  bufio.NewReader(file),
		eof:     false,
	}

	// 检查是否是aof混合rdb,如果是则报错
	var sig [5]byte
	_, err = parser.reader.Read(sig[:])
	if err != nil {
		_ = file.Close()
		return nil, err
	}

	if string(sig[:]) == "REDIS" || string(sig[:]) == "redis" {
		_ = file.Close()
		return nil, kErrorAofWithRdbPremptNotSupported
	}

	// seek回去
	_, err = parser.file.Seek(0, io.SeekStart)
	if err != nil {
		return nil, err
	}

	// file.Seek但是缓冲区已经有数据 因此需要reset以下
	parser.reader.Reset(parser.file)
	return parser, nil
}

func (parser *Parser) Close() error {
	if parser.file != nil {
		return parser.file.Close()
	}
	return nil
}

func (parser *Parser) Eof() bool {
	return parser.eof
}

func (parser *Parser) AofPath() string {
	return parser.aofPath
}

func (parser *Parser) ParseSingleMsg() (string, error) {
	line, _, err := parser.reader.ReadLine()
	if err != nil {
		if err == io.EOF {
			parser.eof = true
		}
		return "", err
	}

	switch line[0] {
	case '*':
		var res string
		arrayLen := parseBulkArrayLen(line)
		for i := 0; i < arrayLen; i++ {
			msg, err := parser.ParseSingleMsg()
			if err != nil {
				return "", nil
			}
			res += " "
			res += msg
		}
		return res, nil
	case '$':
		bulkLen := parseBulkLen(line)
		bulk := make([]byte, bulkLen+2) // \r\n
		_, err := parser.reader.Read(bulk)
		if err != nil {
			return "", errors.New("Read bulk body error")
		}
		return string(bulk[:bulkLen]), nil
	case '+':
		return string(line[1:]), nil
	case '-':
		return string(line[1:]), nil
	case ':':
		return string(line[1:]), nil
	default:
		panic(errors.New("Illegal redis RESP start symbols"))
	}
}

func assert(expr bool) {
	if !expr {
		panic(errors.New("Assertion error"))
	}
}

func parseBulkArrayLen(line []byte) int {
	assert(line[0] == '*')
	i, err := strconv.Atoi(string(line[1:]))
	assert(err == nil)
	return i
}

func parseBulkLen(line []byte) int {
	assert(line[0] == '$')
	i, err := strconv.Atoi(string(line[1:]))
	assert(err == nil)
	return i
}
