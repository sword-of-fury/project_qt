#ifndef BORDERSYSTEM_H
#define BORDERSYSTEM_H

#include <QObject>
#include <QMap>
#include <QSet>
#include <QPoint>
#include <QVector>

class Map;
class Tile;

/**
 * @brief The BorderSystem class handles automatic border tile placement
 * 
 * This class implements the Automagic Border System, which automatically
 * places appropriate border tiles when a tile is placed on the map.
 * It uses a set of rules to determine which border tiles to place
 * based on the surrounding tiles.
 */
class BorderSystem : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent Parent object
     */
    explicit BorderSystem(QObject* parent = nullptr);
    
    /**
     * @brief Set the map to work with
     * @param map Map to work with
     */
    void setMap(Map* map);
    
    /**
     * @brief Get the map
     * @return Map
     */
    Map* getMap() const { return map; }
    
    /**
     * @brief Apply border tiles around a specific tile
     * @param position Position of the tile
     * @param layer Layer of the tile
     * @return True if borders were applied, false otherwise
     */
    bool applyBorders(const QPoint& position, int layer);
    
    /**
     * @brief Apply border tiles to a region
     * @param region Region to apply borders to
     * @param layer Layer to apply borders to
     * @return True if borders were applied, false otherwise
     */
    bool applyBordersToRegion(const QVector<QPoint>& region, int layer);
    
    /**
     * @brief Register a tile as a border source
     * @param tileId ID of the tile
     * @param borderTileIds IDs of the border tiles for this tile
     */
    void registerBorderSource(int tileId, const QVector<int>& borderTileIds);
    
    /**
     * @brief Check if a tile is a border source
     * @param tileId ID of the tile
     * @return True if the tile is a border source, false otherwise
     */
    bool isBorderSource(int tileId) const;
    
    /**
     * @brief Get the border tiles for a source tile
     * @param tileId ID of the source tile
     * @return Vector of border tile IDs
     */
    QVector<int> getBorderTiles(int tileId) const;
    
    /**
     * @brief Enable or disable the border system
     * @param enabled True to enable, false to disable
     */
    void setEnabled(bool enabled);
    
    /**
     * @brief Check if the border system is enabled
     * @return True if enabled, false if disabled
     */
    bool isEnabled() const { return enabled; }

private:
    /**
     * @brief Check if a position needs a border
     * @param position Position to check
     * @param layer Layer to check
     * @param sourceTileId ID of the source tile
     * @return True if the position needs a border, false otherwise
     */
    bool needsBorder(const QPoint& position, int layer, int sourceTileId);
    
    /**
     * @brief Get the appropriate border tile for a position
     * @param position Position to get the border tile for
     * @param layer Layer to get the border tile for
     * @param sourceTileId ID of the source tile
     * @return ID of the border tile
     */
    int getBorderTileId(const QPoint& position, int layer, int sourceTileId);
    
    /**
     * @brief Check surrounding tiles to determine border type
     * @param position Position to check
     * @param layer Layer to check
     * @param sourceTileId ID of the source tile
     * @return Border type index
     */
    int calculateBorderType(const QPoint& position, int layer, int sourceTileId);

    Map* map;
    bool enabled;
    
    // Map of source tile IDs to border tile IDs
    QMap<int, QVector<int>> borderTiles;
    
    // Set of source tile IDs
    QSet<int> sourceTileIds;
};

#endif // BORDERSYSTEM_H
