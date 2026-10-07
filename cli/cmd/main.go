package main

import (
	"fmt"
	"os"

	"cli/internal"
)

const usage = `usage: roze <command> [args]

commands:
  dev            build, then launch the Standalone
  build          configure & build the VST3 and the Standalone
  install        copy the VST3 into your plugin folder
  run            launch the Standalone
  formats        list the formats supported by the engine
  scan <dir>     detect the format of every file in a sample folder
`

func main() {
	if err := run(os.Args[1:]); err != nil {
		fmt.Fprintln(os.Stderr, "roze:", err)
		os.Exit(1)
	}
}

func run(args []string) error {
	if len(args) == 0 {
		fmt.Print(usage)
		return nil
	}

	switch args[0] {
	case "help", "-h", "--help":
		fmt.Print(usage)
		return nil
	case "formats":
		return internal.Formats()
	case "scan":
		if len(args) != 2 {
			return fmt.Errorf("usage: roze scan <dir>")
		}
		return internal.Scan(args[1])
	}

	project, err := internal.FindProject()
	if err != nil {
		return err
	}

	switch args[0] {
	case "dev":
		return internal.Dev(project)
	case "build":
		return internal.Build(project)
	case "install":
		return internal.Install(project)
	case "run":
		return internal.Run(project)
	default:
		return fmt.Errorf("unknown command %q\n\n%s", args[0], usage)
	}
}
