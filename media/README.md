# Froggo mod demonstrations

Real Windows captures recorded on October 23, 2026. These assets are kept on a
separate branch so each Windhawk submission still changes only its mod source.

## Froggo Desktop Ping Panel

![Running Ping Panel](froggo-desktop-ping-panel/idle.png)

![Hovering the first row shows its destination name](froggo-desktop-ping-panel/hover.png)

The compact running widget displays four latency readings and status indicators.
Hovering a row replaces its latency with the destination name. Both captures are
3x upscales of the real desktop corner; `screenshot.png` is the original
unscaled crop.

## Froggo Explorer Emmet Folders

![Recorded folder expansion](froggo-explorer-emmet-folders/demo.gif)

[Watch or download the MP4 recording](froggo-explorer-emmet-folders/demo.mp4).

The native rename editor receives `Project {1..3}/Assets+Exports`. The mod creates
three project folders, each containing `Assets` and `Exports`. The recording then
opens `Project 1` to show its children. All nine resulting directories were
verified on disk. Only a dedicated dummy folder is shown.

## Froggo Explorer Mixed Sorting

Both captures show the same six dummy items, sorted by **Date modified,
descending**, with **Group by: None**. Only `mixDateModified` changes between
captures. Its original value was restored after the comparison.

### Before: mixed sorting off

![Files and folders remain separate](froggo-explorer-mixed-sorting/before.png)

### After: mixed sorting on

![Files and folders interleave by date](froggo-explorer-mixed-sorting/after.png)

Explorer captures exclude the tab strip and navigation sidebar so personal tabs,
pinned folders, and unrelated locations are not included.

## Froggo Toast Tamer

![Toast Tamer behavior illustration](froggo-toast-tamer/illustration.png)

This is a drawn illustration, not a capture. Dummy notifications sent through
WinRT did not produce a visible banner because notifications were silenced on
the recording machine.

## Runtime provenance

The recordings use the existing installed mods. Their saved sources match the
submitted sources apart from Froggo naming, GitHub author metadata, and the
demonstration links added to the submitted documentation.
The installed Ping Panel and Emmet IDs still use their original names;
recording these demonstrations did not install the renamed mods. This source
comparison does not independently establish how the loaded DLLs were built.
