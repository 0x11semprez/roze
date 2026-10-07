package internal

import (
	"fmt"
	"io/fs"
	"path/filepath"
	"slices"
	"strings"
)

// allFormats mirrors RozeAudioRegistry::allFormats in engine/interface/RozeAudioRegistry.hpp.
var allFormats = []string{
	".3gp", ".aa", ".aac", ".aax", ".act", ".aiff", ".alac", ".amr", ".ape", ".au",
	".awb", ".dss", ".dvf", ".flac", ".gsm", ".iklax", ".ivs", ".m4a", ".m4b", ".m4p",
	".mmf", ".movpkg", ".mp1", ".mp2", ".mp3", ".mpc", ".msv", ".nmf", ".ogg", ".oga",
	".mogg", ".opus", ".qoa", ".ra", ".rm", ".raw", ".rf64", ".sln", ".tta", ".voc",
	".vox", ".wav", ".wma", ".wv", ".webm", ".8svx", ".cda",
}

// Formats prints every audio format supported by the engine.
func Formats() error {
	for _, f := range allFormats {
		fmt.Println(f)
	}
	fmt.Printf("%d formats supported\n", len(allFormats))
	return nil
}

// Scan walks a sample folder and prints the format of every supported file.
func Scan(dir string) error {
	supported, skipped := 0, 0
	err := filepath.WalkDir(dir, func(path string, d fs.DirEntry, err error) error {
		if err != nil {
			return err
		}
		if d.IsDir() {
			return nil
		}
		ext := strings.ToLower(filepath.Ext(path))
		if !slices.Contains(allFormats, ext) {
			skipped++
			return nil
		}
		supported++
		fmt.Printf("%-8s %s\n", ext, path)
		return nil
	})
	if err != nil {
		return err
	}
	fmt.Printf("%d supported, %d skipped\n", supported, skipped)
	return nil
}
