package main

import (
	"os"
	"fmt"

        "github.com/spf13/cobra"
)


func main() {
	cmd := &cobra.Command{
		Use: "", // help string
		Short: "", // short description
		Run: func(cmd *cobra.Command, args []string) {
			// default function if no subcmd is provided
			_ = cmd.Help()
			os.Exit(1)
		},
	}

	// basic operations. Store, get, delete
	cmd.AddCommand(NewCmdStore())
	cmd.AddCommand(NewCmdGet())
	cmd.AddCommand(NewCmdDelete())

	// run store as server
	// cmd.AddCommand(NewCmdServe())

	// run store as client
	// cmd.AddCommand(NewCmdClientStore())
	// cmd.AddCommand(NewCmdClientGet())
	// cmd.AddCommand(NewCmdClientDelete())

	err := cmd.Execute()
	if err != nil {
		fmt.Println("error")
	}
}
