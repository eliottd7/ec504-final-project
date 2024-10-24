package main

import (
	"github.com/spf13/cobra"
)

type CmdStore struct {
	DocumentPath string
	StorePath string
}

func (c *CmdStore) Run(cmd *cobra.Command, args []string) error {
	// IMPLEMENTATION OF STORE GOES HERE
	// should update store located in c.StorePath with
	// contents of c.DocumentPath
	return nil
}

func NewCmdStore() *cobra.Command {
	c := &CmdGet{
		DocumentPath: "",
		StorePath: "",
	}

	cmd := &cobra.Command{
		Use: "store", // usage string
		Short: "", // short description
		RunE: c.Run,
	}


	fs := cmd.Flags() // set cli flags
	fs.StringVarP(
		&c.DocumentPath, // location to store cli flag
		"doc", // long form (--docpath)
		"f",  // short form (-f)
		c.DocumentPath, // default value
		"path of file to store",
	)
	fs.StringVarP(
		&c.StorePath,
		"store",
		"d",
		c.StorePath,
		"path of store to use",
	)

	return cmd
}
