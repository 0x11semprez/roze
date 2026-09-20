<p align="center"><img src="docs/logo.png" alt="RÖZE" width="600"></p>

röze is a sampler VST built with JUCE (VST3 & Standalone).<br/>
röze supports its own sampling engine, built from scratch (`engine/`). <br/>
röze supports 47 audio formats out of the box, with automatic format detection.<br/>
röze supports loading, mapping and playing your sounds straight from your DAW. <br/>
röze ships with its own command line tool, written in Go (`cmd/roze/`), so the whole workflow lives in one place. <br/>
<br/>

## build

```bash
cmake -B build && cmake --build build
```

## workflow

röze is not only a plugin: the `roze` command drives the full pipeline, from building the VST to managing your sample library, without leaving the terminal.

```bash
roze build              # configure & build the VST3 and the Standalone
roze install            # copy the VST3 into your plugin folder (~/.vst3)
roze run                # launch the Standalone
roze formats            # list the 47 formats supported by the engine
roze scan <dir>         # detect the format of every file in a sample folder
roze map <dir>          # build a keymap from a sample folder, ready to load in the plugin
```

Every command is native Go code, not a wrapper around shell scripts: the CLI talks to the same engine (`engine/`) that runs inside the plugin, so format detection and mapping behave exactly the same in the terminal and in your DAW.

```
samples/ ──▶ roze scan ──▶ roze map ──▶ röze (VST3 / Standalone) ──▶ your DAW
```
