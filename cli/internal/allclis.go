package internal

import (
	"errors"
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"strconv"
)

// Build configures the project with CMake and builds the VST3 and the Standalone.
func Build(p *Project) error {
	if err := p.run("cmake", "-B", p.BuildDir(), "-DCMAKE_BUILD_TYPE="+buildType); err != nil {
		return err
	}
	return p.run("cmake", "--build", p.BuildDir(), "--config", buildType,
		"--parallel", strconv.Itoa(runtime.NumCPU()))
}

// Install copies the built VST3 bundle into the user plugin folder.
func Install(p *Project) error {
	src := p.VST3Path()
	if _, err := os.Stat(src); err != nil {
		return fmt.Errorf("VST3 not found at %s: run `roze build` first", src)
	}

	pluginDir, err := vst3Dir()
	if err != nil {
		return err
	}
	dst := filepath.Join(pluginDir, filepath.Base(src))

	if err := os.RemoveAll(dst); err != nil {
		return err
	}
	if err := os.CopyFS(dst, os.DirFS(src)); err != nil {
		return err
	}
	fmt.Printf("installed %s\n", dst)
	return nil
}

func vst3Dir() (string, error) {
	home, err := os.UserHomeDir()
	if err != nil {
		return "", err
	}
	switch runtime.GOOS {
	case "darwin":
		return filepath.Join(home, "Library", "Audio", "Plug-Ins", "VST3"), nil
	case "windows":
		return "", errors.New("install is not supported on Windows: copy the VST3 by hand")
	default:
		return filepath.Join(home, ".vst3"), nil
	}
}

// Run launches the Standalone build of the plugin.
func Run(p *Project) error {
	bin := p.StandalonePath()
	if _, err := os.Stat(bin); err != nil {
		return fmt.Errorf("Standalone not found at %s: run `roze build` first", bin)
	}
	return p.run(bin)
}

// Dev builds the project and launches the Standalone.
func Dev(p *Project) error {
	if err := Build(p); err != nil {
		return err
	}
	return Run(p)
}
