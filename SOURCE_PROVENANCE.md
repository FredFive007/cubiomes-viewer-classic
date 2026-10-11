# Source provenance

This is an unofficial compatibility fork, first published in October 2026.
Original copyright and license notices are retained.

| Component | Origin | Source snapshot | License |
| --- | --- | --- | --- |
| Viewer and classic UI | [Cubitect/cubiomes-viewer](https://github.com/Cubitect/cubiomes-viewer) | tag `4.0.1`, commit `73f6f9735b9a8e7701d9acce2577ac55cc5f32d3` | GPL-3.0 |
| Generation engine | [xpple/cubiomes](https://github.com/xpple/cubiomes), derived from Cubitect/cubiomes | archive snapshot `4f04235` | MIT |
| Lua | Upstream Viewer's bundled Lua 5.4.4 | included in `lua/` | MIT |

Downloaded source archives used for this port:

```text
Viewer 4.0.1 zip SHA-256:
b1ff0949ea1f9553f28448dd35254247a46a28f6924f3a353cef8fa5a8dddd25
Engine snapshot zip SHA-256:
637470e1d596b67b5ee69084b80c564675f1ec8148d5bbef56665bbf589a3e94
```

The engine is vendored to make source archives self-contained. Its only local source
edit adds the name `unused_rng` to an unused C function parameter for GCC 8 compatibility.
The port also adapts Viewer calls to updated engine APIs, appends new biome/structure entries,
adds a CMake build and tests, and uses a separate settings namespace.
The 19 original `.ui` files and original `.qss` stylesheet are unchanged; run
`python validation/verify_ui.py` to check their byte hashes.

The public Windows binary uses Qt 5.15.2 and MinGW 8.1.0 with dynamic Qt libraries.
Dependency notices are included in the portable archive's `licenses/` directory.
Matching upstream sources: [Qt 5.15.2](https://download.qt.io/archive/qt/5.15/5.15.2/submodules/)
and [MinGW builds](https://github.com/niXman/mingw-builds).

`validation/vanilla-26.3-biomes.csv` contains generated reference data from an isolated
Minecraft Java 26.3 server with Fabric API and the included `VanillaBiomeProbe` only.
It contains no production world or player data.
