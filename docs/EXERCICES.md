# roze — Comprendre le sampler par la pratique

Ce fichier est un parcours d'exercices sur **ce projet**. Chaque exercice touche
une seule brique. Le résultat se vérifie à l'oreille dans le Standalone :

```
cmake --build build
./build/roze_artefacts/Standalone/roze
```

**Règle d'or :** avant de lancer, écris ce que tu prédis. Puis vérifie. Si tu es
surpris, c'est là que tu apprends.

---

## Rappel : la recette d'un sampler en 6 lignes

```
1. formatManager.registerBasicFormats()   → je sais lire du wav
2. synth.addVoice() × N                    → j'ai N doigts (polyphonie)
3. reader = createReaderFor(wav)           → j'ouvre le fichier
4. synth.addSound(SamplerSound(...))       → quelles notes jouent quel wav
5. prepareToPlay  → setCurrentPlaybackSampleRate(sampleRate)
6. processBlock   → synth.renderNextBlock(buffer, midi, 0, numSamples)
```

| Classe JUCE               | Rôle                                                  | Analogie              |
|---------------------------|-------------------------------------------------------|-----------------------|
| `juce::Synthesiser`       | Reçoit le MIDI, distribue les notes aux voix          | Le piano              |
| `juce::SamplerVoice`      | Une note jouée en même temps (8 voix = 8 notes max)   | Un doigt              |
| `juce::SamplerSound`      | Le wav + la plage de notes + l'enveloppe              | La corde qui vibre    |
| `juce::AudioFormatManager`| Décode les fichiers audio (wav, aiff…)                | Le lecteur            |

---

## Palier 0 — Faire compiler (obligatoire)

### Exercice 0 : corriger les bugs et brancher BinaryData

Le code actuel ne compile pas. À faire toi-même :

1. Trouve un wav court (une note de piano, un "C" chanté…) → `sample/c5.wav`
2. Dans `CMakeLists.txt`, ajoute :
   ```cmake
   juce_add_binary_data(rozeData SOURCES sample/c5.wav)
   target_link_libraries(roze PRIVATE rozeData)
   ```
3. Dans `src/PluginProcessor.cpp`, corrige les 3 fautes :
   - `BinaryData::c5_wav, BinaryData::c5_wav` → le second doit être `BinaryData::c5_wavSize`
   - `midiPlaybackEngine.addSound(sound)` → `synth.addSound(sound)`
   - `setCurrentPlaybackSampleRate(double sampleRate)` → `setCurrentPlaybackSampleRate(sampleRate)`
4. `cmake --build build` jusqu'à zéro erreur, puis joue une note.

*Ce que tu apprends :* lire les erreurs du compilateur, le cycle build → test.
Tu ne comprendras rien tant que tu ne peux pas entendre ce que tu changes.

---

## Palier 1 — Manipuler les paramètres (aucune nouvelle notion)

### Exercice 1.1 : la polyphonie
Mets `numVoices = 1`. Joue un accord. Que se passe-t-il ? Remets 2, puis 3.

*Tu comprends physiquement ce qu'est une voix.*

### Exercice 1.2 : la transposition
Change `originalMidiNote` de 60 à 72, sans toucher au wav. Joue C4.
Pourquoi le son est-il plus grave ? Puis mets 48.

*La note de référence ne change pas le wav, mais la façon dont le synth le
resample pour atteindre la note demandée.*

### Exercice 1.3 : la plage de notes
Mets `midiNoteSet` à `{60}` seulement. Joue D4 : silence.
Puis remplace le vecteur par une boucle `for` qui allume les bits 0 à 127.

*Tu comprends `BigInteger` comme un ensemble de bits, un par note MIDI.*

### Exercice 1.4 : l'enveloppe
- `attack = 1.0` → le son monte lentement
- `release = 2.0` → il traîne après relâchement
- `release = 0.0` → tu entends un "clic" à la fin

*Tu comprends pourquoi le release de 0.1 existait.*

---

## Palier 2 — Comprendre le flux (lire, pas écrire)

### Exercice 2.1 : espionner processBlock
Ajoute au début de `processBlock` :
```cpp
DBG("bloc de " << buffer.getNumSamples() << " samples, "
    << midiMessages.getNumEvents() << " events MIDI");
```
Lance le Standalone depuis le terminal, joue une note. Regarde la fréquence des
lignes et le moment où les events passent de 0 à 1.

*Tu vois la boucle temps-réel de tes propres yeux.*
**Enlève-le après** : un `DBG` dans `processBlock`, c'est pour observer, jamais
pour laisser (ça alloue et ça bloque le thread audio).

### Exercice 2.2 : lire le code JUCE, pas le tien
Ouvre `build/_deps/juce-src/modules/juce_audio_basics/synthesisers/juce_Synthesiser.cpp`.
Lis **uniquement** `Synthesiser::renderNextBlock` puis `Synthesiser::noteOn`.

Réponds par écrit : quand une note arrive et que toutes les voix sont prises,
que fait-il ? (cherche `findFreeVoice` et `findVoiceToSteal`)

*Le synth n'est pas magique : c'est 300 lignes lisibles.*

### Exercice 2.3 : les includes
Retire `#include <juce_audio_basics/juce_audio_basics.h>` du `.h`. Compile.
Lis l'erreur. Remets-le. Fais pareil avec `juce_audio_formats`.

*Tu comprends à quoi sert chaque module JUCE.*

---

## Palier 3 — Étendre (ajouter une brique)

### Exercice 3.1 : deux samples
Embarque un deuxième wav (`sample/g5.wav`, à ajouter dans `juce_add_binary_data`).
Crée un deuxième `SamplerSound` couvrant les notes 68–75 avec `originalMidiNote = 67`.

*C'est exactement ce que fait un vrai sampler multi-échantillonné : chaque wav
couvre une zone du clavier.*

### Exercice 3.2 : factoriser
Tu as maintenant deux blocs quasi identiques dans le constructeur.
Écris une fonction privée :
```cpp
void addSample(const char* data, int size, int rootNote, int lowNote, int highNote);
```
et appelle-la deux fois.

*Premier vrai réflexe de développeur : dès que tu copies-colles, tu extrais.*

### Exercice 3.3 : charger depuis le disque
`void loadSample(const juce::File& file);` est déclaré dans le `.h` mais jamais
implémenté. Implémente-la :
1. `formatManager.createReaderFor(file)`
2. `synth.clearSounds()`
3. `synth.addSound(...)`

Appelle-la depuis le constructeur avec un chemin en dur pour tester.

*Tu comprends la différence entre `BinaryData` (embarqué dans le plugin) et
`juce::File` (disque), et que le `reader` est le même dans les deux cas.*

### Exercice 3.4 : un bouton dans l'éditeur
Dans `PluginEditor`, ajoute un `juce::TextButton` "Load" qui ouvre un
`juce::FileChooser` et appelle `processorRef.loadSample(file)`.

*Tu relies l'UI au moteur audio — dernière brique d'un sampler minimal complet.*

---

## Palier 4 — Comprendre pour de vrai (optionnel, ambitieux)

### Exercice 4 : réécrire SamplerVoice toi-même
Crée `MySamplerVoice : public juce::SynthesiserVoice` avec juste :
- un pointeur vers le buffer du wav
- une position de lecture `double`
- un ratio de pitch (calculé dans `startNote` à partir de la note demandée
  et de la note de référence)

Dans `renderNextBlock`, lis le sample à `position`, ajoute `ratio` à chaque
itération. Pas d'interpolation, pas d'enveloppe.

Ça sonnera crade mais **tu auras écrit un sampler**. Ensuite compare avec
`juce_SamplerVoice.cpp` pour voir ce que tu as sauté (interpolation linéaire,
ADSR, stéréo).

---

## Ordre conseillé

```
0 → 1.1 → 1.2 → 2.1 → 2.2 → 3.1 → 3.2
```

Après ça tu sais refaire ce projet de zéro. Le reste vient quand tu en as envie.

## Suivi

- [ ] 0 — compile et joue une note
- [ ] 1.1 — polyphonie
- [ ] 1.2 — transposition
- [ ] 1.3 — plage de notes
- [ ] 1.4 — enveloppe
- [ ] 2.1 — DBG dans processBlock
- [ ] 2.2 — lu Synthesiser.cpp
- [ ] 2.3 — includes
- [ ] 3.1 — deux samples
- [ ] 3.2 — addSample()
- [ ] 3.3 — loadSample()
- [ ] 3.4 — bouton Load
- [ ] 4 — MySamplerVoice
