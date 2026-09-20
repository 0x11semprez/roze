<p align="center"><img src="docs/logo.png" alt="RÖZE" width="600"></p>

röze is a sampler VST built with JUCE (VST3 & Standalone).<br/>
röze supports its own sampling engine, built from scratch (`engine/`). <br/>
röze supports 47 audio formats out of the box, with automatic format detection.<br/>
röze supports loading, mapping and playing your sounds straight from your DAW. <br/>
röze ships with its own command line tool, written in Go (`cmd/roze/`), so the whole workflow lives in one place. <br/>
<br/>

röze build <br/>

```bash
cmake -B build && cmake --build build
```

röze workflow <br/>
röze is not only a plugin: the `roze` command drives the full pipeline, from building the VST to managing your sample library, without leaving the terminal. <br/>

```bash
roze build              # configure & build the VST3 and the Standalone
roze install            # copy the VST3 into your plugin folder (~/.vst3)
roze run                # launch the Standalone
roze formats            # list the 47 formats supported by the engine
roze scan <dir>         # detect the format of every file in a sample folder
roze map <dir>          # build a keymap from a sample folder, ready to load in the plugin
```
