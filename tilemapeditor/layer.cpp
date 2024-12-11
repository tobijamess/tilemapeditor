#include "layer.h"
#include "editor.h"
#include "tileatlas.h"

TileMap::TileMap(Editor& editor, TileAtlas& tileAtlas) : editor(editor), tileAtlas(tileAtlas) {}

void TileMap::Initialize(int width, int height) {

}

void TileMap::AddLayer(int width, int height) {
    // create a new TileLayer instance and set passed in properties
    TileLayer newLayer;
    newLayer.width = width;
    newLayer.height = height;
    newLayer.isVisible = true;
    newLayer.opacity = 1.0f;
    newLayer.index = layers.size();
    newLayer.layer.resize(height, std::vector<Tile>(width));
    layers.push_back(newLayer); // push the new layer back into the layers vector
    activeLayerIndex = layers.size() - 1;   // set this new layer as the current/active layer
}

void TileMap::AddTile(const sf::Texture& texture, const sf::IntRect& rect, int index, int x, int y) {
    if (activeLayerIndex < 0 || activeLayerIndex >= layers.size()) return;

    TileLayer& currentLayer = layers[activeLayerIndex];

    if (x >= 0 && x < currentLayer.width && y >= 0 && y < currentLayer.height) {
        Tile& tile = currentLayer.layer[y][x];
        tile.index = index;
        tile.sprite.setTexture(texture);
        tile.sprite.setTextureRect(rect);
        tile.sprite.setScale(layerScaleFactor, layerScaleFactor); // apply scale factor to the layer tiles
        tile.sprite.setPosition(x * layerTileSize, y * layerTileSize); // apply position based on the tile size
    }
}

void TileMap::RemoveTile(int x, int y) {

}

void TileMap::SetCurrentLayer(int index) {
    if (index >= 0 && index < layers.size()) {
        activeLayerIndex = index;
        std::cout << "Switched to layer: " << activeLayerIndex << "\n";
    }
    else {
        std::cerr << "Invalid layer index: " << index << "\n";
    }
}

void TileMap::HandleTilePlacement(const sf::Vector2f& mousePos) {
    const TileAtlas::SelectedTile& selectedTile = tileAtlas.GetSelectedTile();
    // if there is no texture selection in the selectedTile struct, exit early
    if (selectedTile.textureRects.empty()) return;
    // convert mouse position to grid coordinates, accounting for zooming and panning
    sf::Vector2f adjustedMousePos = (mousePos + editor.layerViewOffset) / layerScaleFactor;
    // grid position of the placement
    int gridX = static_cast<int>(adjustedMousePos.x / editor.baseTileSize);
    int gridY = static_cast<int>(adjustedMousePos.y / editor.baseTileSize);

    // iterate through all selected tiles
    for (const auto& rect : selectedTile.textureRects) {
        // calculate offset from the top-left corner of the selection
        int offsetX = (rect.left - selectedTile.selectionBounds.left) / editor.baseTileSize;
        int offsetY = (rect.top - selectedTile.selectionBounds.top) / editor.baseTileSize;
        // calculate tile's grid position on the map
        int targetX = gridX + offsetX;
        int targetY = gridY + offsetY;
        // determine tile index in the atlas
        int tileIndex = (rect.top / editor.baseTileSize) *
            (tileAtlas.GetTexture().getSize().x / editor.baseTileSize) +
            (rect.left / editor.baseTileSize);
        // add the tile to the map
        AddTile(tileAtlas.GetTexture(), rect, tileIndex, targetX, targetY);
    }
}

void TileMap::DrawLayerGrid(sf::RenderTarget& target, int index) {
    if (index < 0 || index >= layers.size()) {  // don't try to draw the layer grid if a layer grid has not been created via the ui buttons
        std::cerr << "Invalid layer index for rendering: " << index << "\n";
        return;
    }
    sf::Vector2f offset = editor.layerViewOffset;   // get the offset of the layer view that is updated when panning
    const TileLayer& layer = layers[index]; // get the active TileLayer instance from the layers vector
// iterates over the tiles of the active layer, each tiles position is calculated based on its coordinates in the grid (x * layerTileSize, y * layerTileSize)
    for (int y = 0; y < layer.height; ++y) {
        for (int x = 0; x < layer.width; ++x) {
            sf::Vector2f tilePosition(static_cast<float>(x * layerTileSize), static_cast<float>(y * layerTileSize));
            const Tile& tile = layer.layer[y][x];
            if (tile.index >= 0) {
                sf::Sprite tileSprite = tile.sprite;    // get the sprite from the Tile struct
                tileSprite.setColor(sf::Color(255, 255, 255, static_cast<sf::Uint8>(layer.opacity * 255)));
                tileSprite.setPosition(tilePosition - offset); // adjust the tile position with panning offset
                target.draw(tileSprite);    // draw the tile sprite in position
            }
        }
    }
    sf::RectangleShape line;    // create line shape to draw grid with
    line.setFillColor(sf::Color(100, 100, 100, 150));
    float startX = -offset.x;
    float startY = -offset.y;
    // iterate from the starting offset up to the grids width and height (layer width/height * layer tile size) and draw the grid lines
    for (float x = startX; x <= layer.width * layerTileSize - offset.x; x += layerTileSize) {
        line.setSize(sf::Vector2f(1.f, layer.height * layerTileSize)); // height of the grid
        line.setPosition(x, -offset.y); // position of each drawn line relative to the offset caused by panning
        target.draw(line);
    }
    // same here but for horizontal grid lines
    for (float y = startY; y <= layer.height * layerTileSize - offset.y; y += layerTileSize) {
        line.setSize(sf::Vector2f(layer.width * layerTileSize, 1.f)); // width of the grid
        line.setPosition(-offset.x, y);
        target.draw(line);
    }
}

void TileMap::HandlePanning(sf::Vector2f mousePos, bool isPanning, float deltaTime) {
    static sf::Vector2f lastMousePos = mousePos;
    if (isPanning) {
        if (lastMousePos != mousePos) {
            sf::Vector2f delta = lastMousePos - mousePos;
            editor.GetLayerView().move(delta);  // update camera/view position
            editor.layerViewOffset += delta;    // update the grid offset
            
        }
        lastMousePos = mousePos;    // update last mouse position each frame
    }
    else {
        lastMousePos = mousePos;  // keep it at the current mouse position when panning stops
    }
}

void TileMap::UpdateTileScale(float scaleFactor) {
    layerScaleFactor = scaleFactor;
    layerTileSize = editor.baseTileSize * layerScaleFactor;

    for (TileLayer& layer : layers) {
        for (int y = 0; y < layer.height; ++y) {
            for (int x = 0; x < layer.width; ++x) {
                Tile& tile = layer.layer[y][x];
                if (tile.index >= 0) { // update only valid tiles
                    tile.sprite.setScale(layerScaleFactor, layerScaleFactor);
                    tile.sprite.setPosition(x * layerTileSize, y * layerTileSize);
                }
            }
        }
    }
}

void TileMap::MergeAllLayers(sf::RenderTarget& target, bool showMergedLayers) {
    if (!showMergedLayers) return;  // if showMergedLayers was passed in as false, exit early
    sf::Vector2f offset = editor.layerViewOffset;   // get the offset from panning
    for (int i = 0; i < layers.size(); ++i) {   // loop through the layers vector drawing each layer with half opacity
        if (i == activeLayerIndex) continue;    // when the loop reaches the active layer, skip it as its already drawn
        const TileLayer& layer = layers[i]; // set layer variable to the current layer index the loop is at
        // if (!layer.isVisible) continue; // skip invisible layers
        // loop through the width and height of each current layer index drawing its' sprite tiles at half opacity
        for (int y = 0; y < layer.height; ++y) {
            for (int x = 0; x < layer.width; ++x) {
                const Tile& tile = layer.layer[y][x];   // get the tile at each width & height position from the current layer
                if (tile.index >= 0) {  // only draw valid tiles 
                    sf::Sprite tileSprite = tile.sprite;    // copy the tile sprite at this width and height of the layer being looped through
                    tileSprite.setColor(sf::Color(255, 255, 255, 128)); // set the sprite to half opacity
                    tileSprite.setPosition(
                        (x * layerTileSize) - offset.x,
                        (y * layerTileSize) - offset.y
                    );  // adjust the tile position relative to the offset
                    tileSprite.setScale(layerScaleFactor, layerScaleFactor);    // make sure it scales to the active layer's size if its' been zoomed
                    target.draw(tileSprite);
                }
            }
        }
    }
}