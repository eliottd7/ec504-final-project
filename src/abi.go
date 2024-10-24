package main


type Edit struct {
	Position int64
	Character byte
}

type BaseDocument struct {
	RefCount int
	Offset int64
}

type Document struct {
	Name string
	Edits []Edit
	BaseIdx int
}

type Directory struct {
	Name string
	Documents []Document
	Directories []Directory
}

type DDStoreHeader struct {
	HeaderSize int
	Directories []Directory
	Documents []Document
}
