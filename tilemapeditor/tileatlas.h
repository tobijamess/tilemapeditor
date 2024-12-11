#ifndef TILEATLAS_H
#define TILEATLAS_H

#include <SFML/Graphics.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Texture.hpp>
#include "layer.h"

struct TileAtlas {
    Editor& editor;
    float deltaTime;  // delta time for consistent timing
    float atlasTileSize = 16.0f;     // base tile size (e.g. 16x16)
    sf::Texture textureAtlas; // atlas texture
    sf::Sprite atlasSprite; // atlas sprite
    sf::Vector2f atlasPos = { 0, 0 }; // default atlas position

    bool isSelecting = false;
    sf::Vector2i selectionStartIndices; // drag-selection start
    sf::Vector2i selectionEndIndices;   // drag-selection end

    struct SelectedTile {
        int index = -1;  // index in the atlas
        std::vector<sf::IntRect> textureRects; // all selected tiles' texture regions
        sf::IntRect selectionBounds;    // drag selected area bounds
        sf::Sprite sprite;  // sprite created from texture and texture rect
    };
    SelectedTile selectedTile;

    TileAtlas(Editor& editor);
    bool Initialize();
    void HandleSelection(sf::Vector2f mousePos, bool isDragging, float deltaTime);
    sf::IntRect GetSelectionBounds() const;
    void HandlePanning(sf::Vector2f mousePos, bool isPanning, float deltaTime);
    void UpdateTileSize(float scaleFactor);
    void DrawAtlas(sf::RenderTarget& target);
    void DrawDragSelection(sf::RenderTarget& target);
    // getter functions to return information about the tile e.g. texture of a tile, and a tile at specific atlas index
    const sf::Texture& GetTexture() { return textureAtlas; }
    const SelectedTile GetSelectedTile() const { return selectedTile; }
    void SetSelectedTile(const SelectedTile& tile) { selectedTile = tile; }
};
#endif