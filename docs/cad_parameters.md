# CAD parameters

Dimensions shared between CAD modules, and the masses used for the tipping check. Every value has a source: `cad/layout.SLDPRT`, a datasheet or a measurement, never a guess (`CLAUDE.md`, "Robotics conventions"). How the values are used in SolidWorks: `cad/README.md` and D012.

## Shared dimensions

A dimension goes here when parts in more than one module must agree on it. Each part that uses it has a global variable with the same name. Names are lowercase with underscores.

| Name | Value | Unit | Owner | Used by | Source |
|---|---|---|---|---|---|

## Masses

Each mass is entered in SolidWorks as a mass override (Tools > Evaluate > Mass Properties > Override Mass Properties). For printed parts, use the weighed print or the slicer estimate, because SolidWorks assumes solid parts.

| Part | Mass (g) | Source |
|---|---|---|
