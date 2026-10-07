package internal

import (
	"errors"
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
)

const (
	productName = "roze"
	buildType   = "Release"
)

// Project points to the root of the roze repository.
type Project struct {
	Root string
}

// FindProject walks up from the working directory until it finds the roze root.
func FindProject() (*Project, error) {
	dir, err := os.Getwd()
	if err != nil {
		return nil, err
	}

	for {
		if isProjectRoot(dir) {
			return &Project{Root: dir}, nil
		}
		parent := filepath.Dir(dir)
		if parent == dir {
			return nil, errors.New("roze project not found: run the command inside the repository")
		}
		dir = parent
	}
}

func isProjectRoot(dir string) bool {
	for _, name := range []string{"CMakeLists.txt", "engine", "src"} {
		if _, err := os.Stat(filepath.Join(dir, name)); err != nil {
			return false
		}
	}
	return true
}

func (p *Project) BuildDir() string {
	return filepath.Join(p.Root, "build")
}

func (p *Project) ArtefactsDir() string {
	return filepath.Join(p.BuildDir(), productName+"_artefacts", buildType)
}

func (p *Project) StandalonePath() string {
	name := productName
	switch runtime.GOOS {
	case "darwin":
		return filepath.Join(p.ArtefactsDir(), "Standalone", name+".app", "Contents", "MacOS", name)
	case "windows":
		name += ".exe"
	}
	return filepath.Join(p.ArtefactsDir(), "Standalone", name)
}

func (p *Project) VST3Path() string {
	return filepath.Join(p.ArtefactsDir(), "VST3", productName+".vst3")
}

// run executes a command from the project root and streams its output.
func (p *Project) run(name string, args ...string) error {
	cmd := exec.Command(name, args...)
	cmd.Dir = p.Root
	cmd.Stdin = os.Stdin
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr
	if err := cmd.Run(); err != nil {
		return fmt.Errorf("%s failed: %w", name, err)
	}
	return nil
}
