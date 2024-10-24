package main

import (
	"github.com/spf13/cobra"
)

type CmdGet struct {
	DocumentPath string
	OutPath string
	StorePath string
}

func (c *CmdGet) Run(cmd *cobra.Command, args []string) error {
	// IMPLEMENTATION OF GET GOES HERE
	// should write contents of c.StorePath:c.DocumentPath
	// to c.OutPath
	return nil
}

func NewCmdGet() *cobra.Command {
	c := &CmdGet{
		DocumentPath: "",
		StorePath: "",
	}

	cmd := &cobra.Command{
		Use: "get", // usage string
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
	fs.StringVarP(
		&c.OutPath,
		"output",
		"o",
		c.OutPath,
		"path to write to",
	)

	return cmd
}
