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
- **Few links between parts:** use `layout.SLDPRT` to decide the key dimensions. Avoid in-context references between parts, because they break easily when files move or two people edit.
- **Real masses:** the tipping check (Must) needs real masses. Weigh each purchased part and enter the result in Tools > Evaluate > Mass Properties > Override Mass Properties. Printed parts are not solid, so use the weighed print or the slicer estimate. Record each mass and its source in `docs/`.
- **One SolidWorks version:** a file saved in a newer version does not open for editing in an older one. Both team members use the same version, also on the lab computers.
- **Backups outside the repository:** in Tools > Options > Backup/Recover, set the backup folder and the auto-recover folder to a local folder outside the repository, and turn off saving backups in the same location as the original.
- **STEP exports:** export the top assembly to `step/` at each CAD milestone (v0, v1, v2), not after every change. STEP files are not in Git LFS, so the 3000 KB limit of the pre-commit hook applies to them.
