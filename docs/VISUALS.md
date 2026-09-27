# Visual assets

README previews use the firmware's5×7 glyph tables and representative fixtures. They are not photographs or captured telescope telemetry. Values are fictional; addresses use TEST-NET ranges.

```sh
make previews
make quality
```

`scripts/render-previews.py` uses Python's standard library and emits an SVG raster of firmware glyphs with a simulated-data caption. It does not reproduce every runtime state or animation. CI checks output freshness.

The banner is original code-drawn vector artwork. No third-party logo or recording is bundled. Review display contents, labels, backgrounds and metadata before adding hardware photographs.
