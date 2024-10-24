package main

import (
	"github.com/spf13/cobra"
)

type CmdDelete struct {
	DocumentPath string
	StorePath string
}

func (c *CmdDelete) Run(cmd *cobra.Command, args []string) error {
	// IMPLEMENTATION OF DELETE GOES HERE
	// should remove c.DocumentPath from c.StorePath
	return nil
}

func NewCmdDelete() *cobra.Command {
	c := &CmdStore{
		DocumentPath: "",
		StorePath: "",
	}

	cmd := &cobra.Command{
		Use: "delete", // usage string
		Short: "", // short description
		RunE: c.Run,
	}


	fs := cmd.Flags() // set cli flags
	fs.StringVarP(
		&c.DocumentPath, // location to store cli flag
		"doc", // long form (--docpath)
		"f",  // short form (-f)
		c.DocumentPath, // default value
		"path of file to retrieve",
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
