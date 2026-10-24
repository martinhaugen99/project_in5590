# CAD

SolidWorks source for M.A.T.S. One part file per part, one subassembly per module, and one top assembly that holds only the subassemblies.

## Layout

```
cad/
├── mats.SLDASM              # top assembly: only subassemblies, mated
├── layout.SLDPRT            # CAD v0: side-view sketches of tracks, arm pivots, bucket reach, sensors
├── body/
│   ├── body.SLDASM
│   └── body_chassis.SLDPRT
├── track_drive/             # one side, mirrored in the top assembly
│   ├── track_drive.SLDASM
│   ├── track_sprocket_drive.SLDPRT
│   └── track_sprocket_idler.SLDPRT
├── brush_arm/               # same pattern
├── bucket_arms/
├── head/
├── purchased/               # vendor models: motors, encoders, servos, actuator, sensors, boards, display
└── step/                    # STEP export of the top assembly at each milestone: mats_v0.step, ...
```

Print files go in `print/<part name>/`: the `.3mf` file and a `settings.md` with printer, material, nozzle, layer height, infill, orientation, supports and tested clearances.

## Owners

Each module has one owner, who creates and edits all parts and the subassembly in its folder (D012). The other person may open these files but does not save them. Fill in the table when the owners are decided.

| Files | Owner |
|---|---|
| `mats.SLDASM`, `layout.SLDPRT`, `step/` | Martin |
| `body/` | not decided |
| `track_drive/` | not decided |
| `brush_arm/` | not decided |
| `bucket_arms/` | not decided |
| `head/` | not decided |
| `purchased/` | anyone; models rarely change after they are added |

Adding or changing parts inside a subassembly does not change `mats.SLDASM`. The top assembly changes only when a subassembly is added or the mates between modules change, and Martin makes those changes.

## Shared dimensions

Dimensions that more than one module depends on, such as the bucket shaft diameter and its pivot position, are listed once in [`docs/cad_parameters.md`](../docs/cad_parameters.md) (D012). There is no linked equation file: SolidWorks stores the full path to it, so the link breaks in a clone in another folder.

- In each part, add the shared values it uses as global variables (Tools > Equations > Global Variables), with the same names as in the table, and drive the dimensions from them.
- The values come from `layout.SLDPRT`, datasheets or measurements, never from guesses.
- To change a shared value, its owner updates the table, commits and pushes it, and tells the other person. Each owner then updates the global variable in their own parts. Check the fit with Interference Detection (Evaluate tab) in the top assembly.

## Setup

Run once per clone, from the repository root. Git for Windows already includes Git LFS.

```bash
git lfs install --local
```

## Editing a file: lock, edit, push, unlock

SolidWorks files (`.SLDPRT`, `.SLDASM`, `.SLDDRW`) are stored in Git LFS as lockable files (D011). They stay read-only until you lock them, so only one person can edit a file at a time. Git cannot merge two versions of a SolidWorks file.

```bash
git pull                                                  # close the files in SolidWorks first
git lfs lock cad/track_drive/track_sprocket_drive.SLDPRT  # the file becomes writable
# edit and save in SolidWorks, then close the file
git add cad/track_drive/track_sprocket_drive.SLDPRT
git commit -m "Widen drive sprocket teeth"
git push
git lfs unlock cad/track_drive/track_sprocket_drive.SLDPRT
git lfs locks                                             # who has locked what
```

- Lock before you edit. A file you have not locked opens read-only in SolidWorks. Do not remove the read-only flag by hand.
- Push before you unlock. `git lfs unlock` refuses a file with uncommitted changes. If you unlock before pushing, the other person can lock and edit the old version.
- An assembly is a file too. Lock it before you add parts, change mates or move components. When SolidWorks asks to save an assembly you have not locked, choose not to save it.
- A new file needs no lock before its first commit. The commit makes it read-only, so lock it before the next edit.
- Do not edit the same file on two branches.
- Close the files in SolidWorks before `git pull`, `git checkout` or `git switch`.

## Rules

- **Names:** lowercase with underscores, starting with the module name (`track_sprocket_drive.SLDPRT`). Do not put versions or dates in file names; git keeps the history.
- **Rename and move only from inside SolidWorks:** rename a component in the FeatureManager design tree, or use Pack and Go for many files. Both update the references. Renaming or moving in Explorer or with `git mv` breaks the assemblies.
- **Mirror the track drive:** model one side and mirror it in the top assembly, so a change applies to both sides.
- **No links between parts:** avoid in-context references between parts. They break easily when files move, and they tie your parts to files that someone else owns. Share dimensions through the table instead.
- **Real masses:** the tipping check (Must) needs real masses. Weigh each purchased part and enter the result in Tools > Evaluate > Mass Properties > Override Mass Properties. Printed parts are not solid, so use the weighed print or the slicer estimate. Record each mass and its source in `docs/cad_parameters.md`.
- **One SolidWorks version:** a file saved in a newer version does not open for editing in an older one. Both team members use the same version, also on the lab computers.
- **Backups outside the repository:** in Tools > Options > Backup/Recover, set the backup folder and the auto-recover folder to a local folder outside the repository, and turn off saving backups in the same location as the original.
- **STEP exports:** export the top assembly to `step/` at each CAD milestone (v0, v1, v2), not after every change. STEP files are not in Git LFS, so the 3000 KB limit of the pre-commit hook applies to them.
