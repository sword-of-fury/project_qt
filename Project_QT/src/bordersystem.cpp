#include "bordersystem.h"
#include "map.h"
#include "tile.h"
#include <QDebug>

BorderSystem::BorderSystem(QObject* parent)
    : QObject(parent)
    , map(nullptr)
    , enabled(true)
{
}

void BorderSystem::setMap(Map* map)
{
    this->map = map;
}

bool BorderSystem::applyBorders(const QPoint& position, int layer)
{
    if (!map || !enabled) {
        return false;
    }
    
    // Get the tile at the position
    Tile* tile = map->getTile(position);
    if (!tile) {
        return false;
    }
    
    // Get the tile ID
    int tileId = tile->getId();
    
    // Check if this tile is a border source
    if (!isBorderSource(tileId)) {
        return false;
    }
    
    // Check surrounding tiles and apply borders
    bool bordersApplied = false;
    
    // Define the 8 surrounding positions
    const QPoint surroundingOffsets[] = {
        QPoint(-1, -1), QPoint(0, -1), QPoint(1, -1),
        QPoint(-1, 0),                 QPoint(1, 0),
        QPoint(-1, 1),  QPoint(0, 1),  QPoint(1, 1)
    };
    
    for (const QPoint& offset : surroundingOffsets) {
        QPoint checkPos = position + offset;
        
        // Check if this position needs a border
        if (needsBorder(checkPos, layer, tileId)) {
            // Get the appropriate border tile ID
            int borderTileId = getBorderTileId(checkPos, layer, tileId);
            
            if (borderTileId > 0) {
                // Apply the border tile
                Tile* borderTile = new Tile(borderTileId);
                map->setTile(checkPos, layer, borderTile);
                bordersApplied = true;
            }
        }
    }
    
    return bordersApplied;
}

bool BorderSystem::applyBordersToRegion(const QVector<QPoint>& region, int layer)
{
    if (!map || !enabled || region.isEmpty()) {
        return false;
    }
    
    bool bordersApplied = false;
    
    // Apply borders to each position in the region
    for (const QPoint& position : region) {
        if (applyBorders(position, layer)) {
            bordersApplied = true;
        }
    }
    
    return bordersApplied;
}

void BorderSystem::registerBorderSource(int tileId, const QVector<int>& borderTileIds)
{
    borderTiles[tileId] = borderTileIds;
    sourceTileIds.insert(tileId);
}

bool BorderSystem::isBorderSource(int tileId) const
{
    return sourceTileIds.contains(tileId);
}

QVector<int> BorderSystem::getBorderTiles(int tileId) const
{
    return borderTiles.value(tileId, QVector<int>());
}

void BorderSystem::setEnabled(bool enabled)
{
    this->enabled = enabled;
}

bool BorderSystem::needsBorder(const QPoint& position, int layer, int sourceTileId)
{
    if (!map) {
        return false;
    }
    
    // Get the tile at the position
    Tile* tile = map->getTile(position);
    
    // If there's no tile or the tile is already a border tile, no need for a border
    if (!tile || tile->getId() == 0) {
        return true; // Empty position needs a border
    }
    
    // Check if the tile is the same as the source tile
    if (tile->getId() == sourceTileId) {
        return false; // Same tile type, no need for border
    }
    
    // Check if the tile is already a border tile for this source
    QVector<int> borders = getBorderTiles(sourceTileId);
    if (borders.contains(tile->getId())) {
        return false; // Already a border tile
    }
    
    // Otherwise, this position needs a border
    return true;
}

int BorderSystem::getBorderTileId(const QPoint& position, int layer, int sourceTileId)
{
    // Calculate the border type based on surrounding tiles
    int borderType = calculateBorderType(position, layer, sourceTileId);
    
    // Get the border tiles for this source
    QVector<int> borders = getBorderTiles(sourceTileId);
    
    // If there are no border tiles or the border type is invalid, return 0
    if (borders.isEmpty() || borderType < 0 || borderType >= borders.size()) {
        return 0;
    }
    
    // Return the appropriate border tile ID
    return borders[borderType];
}

int BorderSystem::calculateBorderType(const QPoint& position, int layer, int sourceTileId)
{
    if (!map) {
        return -1;
    }
    
    // Define the 8 surrounding positions in clockwise order starting from top
    const QPoint surroundingOffsets[] = {
        QPoint(0, -1),  // North
        QPoint(1, -1),  // Northeast
        QPoint(1, 0),   // East
        QPoint(1, 1),   // Southeast
        QPoint(0, 1),   // South
        QPoint(-1, 1),  // Southwest
        QPoint(-1, 0),  // West
        QPoint(-1, -1)  // Northwest
    };
    
    // Create an 8-bit mask where each bit represents whether the corresponding
    // surrounding position contains the source tile (1) or not (0)
    int mask = 0;
    
    for (int i = 0; i < 8; ++i) {
        QPoint checkPos = position + surroundingOffsets[i];
        Tile* tile = map->getTile(checkPos);
        
        if (tile && tile->getId() == sourceTileId) {
            // Set the corresponding bit in the mask
            mask |= (1 << i);
        }
    }
    
    // Use the mask to determine the border type
    // This is a simplified version; a real implementation would have
    // a more complex mapping from mask to border type
    
    // For now, we'll just use the mask as an index into the border tiles array
    // In a real implementation, you'd map the mask to specific border types
    // based on your border tile set
    
    // For example, if you have 16 border tiles, you might map the mask to
    // an index between 0 and 15 based on the pattern of surrounding tiles
    
    // For this simplified version, we'll just return the mask modulo the number of border tiles
    QVector<int> borders = getBorderTiles(sourceTileId);
    if (borders.isEmpty()) {
        return -1;
    }
    
    return mask % borders.size();
}
