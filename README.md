harbour-mapp
============

Slippy map component/app for SailfishOS

This projects goal has two parts. First, make a SlippyMap{} qml component, that can be used anywhere, and secondly, to become a usefull offline routing app.

The structure is basically very simple. There is the SlippyMap component, which uses a SlippyProvider and a SlippyCache. 
It will request all the tiles it needs from the slippycache, which will get them from the SlippyProvider if it doesn't have it.

To add another source of maps, simply extend slippyProvider, implement the virtual functions. Providers can also provide overlays for maps, such as OpenSeaMap, which will be rendered on top of another map.