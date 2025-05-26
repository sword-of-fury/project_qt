: MapCommand(parent)
    , map(map)
    , pastePos(pastePos)
    , clipboardData(clipboardData)
{
    setText(QObject::tr("Paste Selection"));

    // Store items that will be replaced during redo for undo
    QJsonArray tilesArray = clipboardData["tiles"].toArray();
    for (const QJsonValue& value : tilesArray) {
        QJsonObject tileObj = value.toObject();
        int relX = tileObj["x"].toInt();
        int relY = tileObj["y"].toInt();
        int z = tileObj["z"].toInt();

        QPoint targetTile(pastePos.x() + relX, pastePos.y() + relY);

        if (targetTile.x() >= 0 && targetTile.x() < map->getWidth() &&
            targetTile.y() >= 0 && targetTile.y() < map->getHeight() &&
            z >= 0 && z < 16)
        {
            Tile* tile = map->getTile(targetTile.x(), targetTile.y(), z);
            if (tile && !tile->getItems().isEmpty()) {
                replacedItems.append({targetTile, {z, tile->getItems()}});
            }
        }
    }
}

void PasteSelectionCommand::redo() {
    processTiles(true); // Call helper to perform pasting
    if (map) {
        map->updateViews();
    }
}

void PasteSelectionCommand::undo() {
    processTiles(false); // Call helper to perform clearing and restoring replaced items
    if (map) {
        map->updateViews();
    }
}

void PasteSelectionCommand::processTiles(bool pasting) {
    if (!map || clipboardData.isEmpty()) {
        return;
    }

    if (pasting) {
        // Clear existing items and paste new ones
        QJsonArray tilesArray = clipboardData["tiles"].toArray();
        for (const QJsonValue& value : tilesArray) {
            QJsonObject tileObj = value.toObject();
            int relX = tileObj["x"].toInt();
            int relY = tileObj["y"].toInt();
            int z = tileObj["z"].toInt();

            QPoint targetTile(pastePos.x() + relX, pastePos.y() + relY);

            if (targetTile.x() >= 0 && targetTile.x() < map->getSize().width() &&
                targetTile.y() >= 0 && targetTile.y() < map->getSize().height() &&
                z >= 0 && z < Map::LayerCount)
            {
                map->clearItems(targetTile.x(), targetTile.y(), z);

                QJsonArray itemsArray = tileObj["items"].toArray();
                for (const QJsonValue& itemValue : itemsArray) {
                    QJsonObject itemObj = itemValue.toObject();
                    int id = itemObj["id"].toInt();
                    int count = itemObj["count"].toInt();
                    // TODO: Wczytaj inne właściwości przedmiotu jeśli zostały zapisane

                    Item newItem(id, ""); // Use ItemManager::getItem() in the future
                    newItem.setCount(count);

                    map->addItem(targetTile.x(), targetTile.y(), z, newItem);
                }
            }
        }
    } else {
        // Clear the pasted items and restore replaced items
        // First, clear the area where items were pasted
        QJsonArray tilesArray = clipboardData["tiles"].toArray();
        for (const QJsonValue& value : tilesArray) {
            QJsonObject tileObj = value.toObject();
            int relX = tileObj["x"].toInt();
            int relY = tileObj["y"].toInt();
            int z = tileObj["z"].toInt();

            QPoint targetTile(pastePos.x() + relX, pastePos.y() + relY);

            if (targetTile.x() >= 0 && targetTile.x() < map->getSize().width() &&
                targetTile.y() >= 0 && targetTile.y() < map->getSize().height() &&
                z >= 0 && z < Map::LayerCount)
            {
                map->clearItems(targetTile.x(), targetTile.y(), z);
            }
        }

        // Then, restore the items that were replaced
        for (const auto& pair : replacedItems) {
            QPoint tilePos = pair.first;
            int z = pair.second.first;
            const QList<Item>& itemsToRestore = pair.second.second;

            if (tilePos.x() >= 0 && tilePos.x() < map->getSize().width() &&
                tilePos.y() >= 0 && tilePos.y() < map->getSize().height() &&
                z >= 0 && z < Map::LayerCount)
            {
                // Assuming clearItems is called before adding items to avoid duplicates
                // map->clearItems(tilePos.x(), tilePos.y(), z); // Already cleared above
                for (const Item& item : itemsToRestore) {
                    map->addItem(tilePos.x(), tilePos.y(), z, item);
                }
            }
        }
    }
}].toArray();
        for (const QJsonValue& value : tilesArray) {
            QJsonObject tileObj = value.toObject();
            int relX = tileObj["x"].toInt();
            int relY = tileObj["y"].toInt();
            int z = tileObj["z"].toInt();

            QPoint targetTile(pastePos.x() + relX, pastePos.y() + relY);

            if (targetTile.x() >= 0 && targetTile.x() < map->getSize().width() &&
                targetTile.y() >= 0 && targetTile.y() < map->getSize().height() &&
                z >= 0 && z < Map::LayerCount)
            {
                map->clearItems(targetTile.x(), targetTile.y(), z);

                QJsonArray itemsArray = tileObj["items"].toArray();
                for (const QJsonValue& itemValue : itemsArray) {
                    QJsonObject itemObj = itemValue.toObject();
                    int id = itemObj["id"].toInt();
                    int count = itemObj["count"].toInt();
                    // TODO: Wczytaj inne właściwości przedmiotu jeśli zostały zapisane

                    Item newItem(id, ""); // Use ItemManager::getItem() in the future
                    newItem.setCount(count);

                    map->addItem(targetTile.x(), targetTile.y(), z, newItem);
                }
            }
        }
    } else {
        // Clear the pasted items and restore replaced items
        // First, clear the area where items were pasted
        QJsonArray tilesArray = clipboardData["tiles"].toArray();
        for (const QJsonValue& value : tilesArray) {
            QJsonObject tileObj = value.toObject();
            int relX = tileObj["x"].toInt();
            int relY = tileObj["y"].toInt();
            int z = tileObj["z"].toInt();

            QPoint targetTile(pastePos.x() + relX, pastePos.y() + relY);

            if (targetTile.x() >= 0 && targetTile.x() < map->getSize().width() &&
                targetTile.y() >= 0 && targetTile.y() < map->getSize().height() &&
                z >= 0 && z < Map::LayerCount)
            {
                map->clearItems(targetTile.x(), targetTile.y(), z);
            }
        }

        // Then, restore the items that were replaced
        for (const auto& pair : replacedItems) {
            QPoint tilePos = pair.first;
            int z = pair.second.first;
            const QList<Item>& itemsToRestore = pair.second.second;

            if (tilePos.x() >= 0 && tilePos.x() < map->getSize().width() &&
                tilePos.y() >= 0 && tilePos.y() < map->getSize().height() &&
                z >= 0 && z < Map::LayerCount)
            {
                // Assuming clearItems is called before adding items to avoid duplicates
                // map->clearItems(tilePos.x(), tilePos.y(), z); // Already cleared above
                for (const Item& item : itemsToRestore) {
                    map->addItem(tilePos.x(), tilePos.y(), z, item);
                }
            }
        }
    }
}

PasteSelectionCommand::PasteSelectionCommand(Map* map, const QPoint& pastePos, const ClipboardData& clipboardData, QUndoCommand* parent)
    : MapCommand(parent),
      map(map),
      pastePos(pastePos),
      clipboardData(clipboardData)
{
    // Store replaced items for undo
    for (int x = pastePos.x(); x < pastePos.x() + clipboardData.width(); ++x) {
        for (int y = pastePos.y(); y < pastePos.y() + clipboardData.height(); ++y) {
            for (int z = 0; z < Map::LayerCount; ++z) {
                if (x >= 0 && x < map->getSize().width() &&
                    y >= 0 && y < map->getSize().height() &&
                    z >= 0 && z < Map::LayerCount) // Ensure z is within bounds
                {
                    map->clearItems(targetTile.x(), targetTile.y(), z);

                    QJsonArray itemsArray = tileObj["items"].toArray();
                    for (const QJsonValue& itemValue : itemsArray) {
                        QJsonObject itemObj = itemValue.toObject();
                        int id = itemObj["id"].toInt();
                        int count = itemObj["count"].toInt();
                        // TODO: Wczytaj inne właściwości przedmiotu jeśli zostały zapisane

                        Item newItem(id, ""); // Use ItemManager::getItem() in the future
                        newItem.setCount(count);

                        map->addItem(targetTile.x(), targetTile.y(), z, newItem);
                    }
                }
            }
        }
    }
    map->updateViews();
}
{
    // Store items that will be replaced during redo for undo
    QJsonArray tilesArray = clipboardData["tiles"].toArray();
    for (const QJsonValue& value : tilesArray) {
        QJsonObject tileObj = value.toObject();
        int relX = tileObj["x"].toInt();
        int relY = tileObj["y"].toInt();
        int z = tileObj["z"].toInt();

        QPoint targetTile(pastePos.x() + relX, pastePos.y() + relY);

        if (targetTile.x() >= 0 && targetTile.x() < map->getWidth() &&
            targetTile.y() >= 0 && targetTile.y() < map->getHeight() &&
            z >= 0 && z < 16)
        {
            Tile* tile = map->getTile(targetTile.x(), targetTile.y(), z);
            if (tile && !tile->getItems().isEmpty()) {
                replacedItems.append({targetTile, {z, tile->getItems()}});
            }
        }
    }
}
{
    // Clear the area where items were pasted
    QJsonArray tilesArray = clipboardData["tiles"].toArray();
    for (const QJsonValue& value : tilesArray) {
        QJsonObject tileObj = value.toObject();
        int relX = tileObj["x"].toInt();
        int relY = tileObj["y"].toInt();
        int z = tileObj["z"].toInt();

        QPoint targetTile(pastePos.x() + relX, pastePos.y() + relY);

        if (targetTile.x() >= 0 && targetTile.x() < map->getWidth() &&
            targetTile.y() >= 0 && targetTile.y() < map->getHeight() &&
            z >= 0 && z < 16)
        {
            map->clearItems(targetTile.x(), targetTile.y(), z);
        }
    }

    // Re-add the items that were replaced
    for (const auto& pair : replacedItems) {
        QPoint pos = pair.first;
        int layer = pair.second.first;
        const QList<Item>& items = pair.second.second;
        for (const Item& item : items) {
            map->addItem(pos.x(), pos.y(), layer, item);
        }
    }
    map->updateViews();
}
{
    // Clear the area where items will be pasted (this was done in undo, but redo needs to ensure it's clear)
    // Note: The constructor already stored replaced items, so we just need to clear and paste.
    QJsonArray tilesArray = clipboardData["tiles"].toArray();
    for (const QJsonValue& value : tilesArray) {
        QJsonObject tileObj = value.toObject();
        int relX = tileObj["x"].toInt();
        int relY = tileObj["y"].toInt();
        int z = tileObj["z"].toInt();

        QPoint targetTile(pastePos.x() + relX, pastePos.y() + relY);

        if (targetTile.x() >= 0 && targetTile.x() < map->getWidth() &&
            targetTile.y() >= 0 && targetTile.y() < map->getHeight() &&
            z >= 0 && z < 16)
        {
            map->clearItems(targetTile.x(), targetTile.y(), z);

            QJsonArray itemsArray = tileObj["items"].toArray();
            for (const QJsonValue& itemValue : itemsArray) {
                QJsonObject itemObj = itemValue.toObject();
                int id = itemObj["id"].toInt();
                int count = itemObj["count"].toInt();
                // TODO: Wczytaj inne właściwości przedmiotu jeśli zostały zapisane

                Item newItem(id, ""); // Use ItemManager::getItem() in the future
                newItem.setCount(count);

                map->addItem(targetTile.x(), targetTile.y(), z, newItem);
            }
        }
    }
    map->updateViews();
}
{
    // Store items that will be replaced during redo for undo
    QJsonArray tilesArray = clipboardData["tiles"].toArray();
    for (const QJsonValue& value : tilesArray) {
        QJsonObject tileObj = value.toObject();
        int relX = tileObj["x"].toInt();
        int relY = tileObj["y"].toInt();
        int z = tileObj["z"].toInt();

        QPoint targetTile(pastePos.x() + relX, pastePos.y() + relY);

        if (targetTile.x() >= 0 && targetTile.x() < map->getWidth() &&
            targetTile.y() >= 0 && targetTile.y() < map->getHeight() &&
            z >= 0 && z < 16)
        {
            Tile* tile = map->getTile(targetTile.x(), targetTile.y(), z);
            if (tile && !tile->getItems().isEmpty()) {
                replacedItems.append({targetTile, {z, tile->getItems()}});
            }
        }
    }
}
{
    // Clear the area where items were pasted
    QJsonArray tilesArray = clipboardData["tiles"].toArray();
    for (const QJsonValue& value : tilesArray) {
        QJsonObject tileObj = value.toObject();
        int relX = tileObj["x"].toInt();
        int relY = tileObj["y"].toInt();
        int z = tileObj["z"].toInt();

        QPoint targetTile(pastePos.x() + relX, pastePos.y() + relY);

        if (targetTile.x() >= 0 && targetTile.x() < map->getWidth() &&
            targetTile.y() >= 0 && targetTile.y() < map->getHeight() &&
            z >= 0 && z < 16)
        {
            map->clearItems(targetTile.x(), targetTile.y(), z);
        }
    }

    // Re-add the items that were replaced
    for (const auto& pair : replacedItems) {
        QPoint pos = pair.first;
        int layer = pair.second.first;
        const QList<Item>& items = pair.second.second;
        for (const Item& item : items) {
            map->addItem(pos.x(), pos.y(), layer, item);
        }
    }
    map->updateViews();
}
{
    // Clear the area where items will be pasted (this was done in undo, but redo needs to ensure it's clear)
    // Note: The constructor already stored replaced items, so we just need to clear and paste.
    QJsonArray tilesArray = clipboardData["tiles"].toArray();
    for (const QJsonValue& value : tilesArray) {
        QJsonObject tileObj = value.toObject();
        int relX = tileObj["x"].toInt();
        int relY = tileObj["y"].toInt();
        int z = tileObj["z"].toInt();

        QPoint targetTile(pastePos.x() + relX, pastePos.y() + relY);

        if (targetTile.x() >= 0 && targetTile.x() < map->getWidth() &&
            targetTile.y() >= 0 && targetTile.y() < map->getHeight() &&
            z >= 0 && z < 16)
        {
            map->clearItems(targetTile.x(), targetTile.y(), z);

            QJsonArray itemsArray = tileObj["items"].toArray();
            for (const QJsonValue& itemValue : itemsArray) {
                QJsonObject itemObj = itemValue.toObject();
                int id = itemObj["id"].toInt();
                int count = itemObj["count"].toInt();
                // TODO: Wczytaj inne właściwości przedmiotu jeśli zostały zapisane

                Item newItem(id, ""); // Use ItemManager::getItem() in the future
                newItem.setCount(count);

                map->addItem(targetTile.x(), targetTile.y(), z, newItem);
            }
        }
    }
    map->updateViews();
}
{
    // Store items that will be replaced during redo for undo
    QJsonArray tilesArray = clipboardData["tiles"].toArray();
    for (const QJsonValue& value : tilesArray) {
        QJsonObject tileObj = value.toObject();
        int relX = tileObj["x"].toInt();
        int relY = tileObj["y"].toInt();
        int z = tileObj["z"].toInt();

        QPoint targetTile(pastePos.x() + relX, pastePos.y() + relY);

        if (targetTile.x() >= 0 && targetTile.x() < map->getWidth() &&
            targetTile.y() >= 0 && targetTile.y() < map->getHeight() &&
            z >= 0 && z < 16)
        {
            Tile* tile = map->getTile(targetTile.x(), targetTile.y(), z);
            if (tile && !tile->getItems().isEmpty()) {
                replacedItems.append({targetTile, {z, tile->getItems()}});
            }
        }
    }
}
{
    // Clear the area where items were pasted
    QJsonArray tilesArray = clipboardData["tiles"].toArray();
    for (const QJsonValue& value : tilesArray) {
        QJsonObject tileObj = value.toObject();
        int relX = tileObj["x"].toInt();
        int relY = tileObj["y"].toInt();
        int z = tileObj["z"].toInt();

        QPoint targetTile(pastePos.x() + relX, pastePos.y() + relY);

        if (targetTile.x() >= 0 && targetTile.x() < map->getWidth() &&
            targetTile.y() >= 0 && targetTile.y() < map->getHeight() &&
            z >= 0 && z < 16)
        {
            map->clearItems(targetTile.x(), targetTile.y(), z);
        }
    }

    // Re-add the items that were replaced
    for (const auto& pair : replacedItems) {
        QPoint pos = pair.first;
        int layer = pair.second.first;
        const QList<Item>& items = pair.second.second;
        for (const Item& item : items) {
            map->addItem(pos.x(), pos.y(), layer, item);
        }
    }
    map->updateViews();
}
{
    // Clear the area where items will be pasted (this was done in undo, but redo needs to ensure it's clear)
    // Note: The constructor already stored replaced items, so we just need to clear and paste.
    QJsonArray tilesArray = clipboardData["tiles"].toArray();
    for (const QJsonValue& value : tilesArray) {
        QJsonObject tileObj = value.toObject();
        int relX = tileObj["x"].toInt();
        int relY = tileObj["y"].toInt();
        int z = tileObj["z"].toInt();

        QPoint targetTile(pastePos.x() + relX, pastePos.y() + relY);

        if (targetTile.x() >= 0 && targetTile.x() < map->getWidth() &&
            targetTile.y() >= 0 && targetTile.y() < map->getHeight() &&
            z >= 0 && z < 16)
        {
            map->clearItems(targetTile.x(), targetTile.y(), z);

            QJsonArray itemsArray = tileObj["items"].toArray();
            for (const QJsonValue& itemValue : itemsArray) {
                QJsonObject itemObj = itemValue.toObject();
                int id = itemObj["id"].toInt();
                int count = itemObj["count"].toInt();
                // TODO: Wczytaj inne właściwości przedmiotu jeśli zostały zapisane

                Item newItem(id, ""); // Use ItemManager::getItem() in the future
                newItem.setCount(count);

                map->addItem(targetTile.x(), targetTile.y(), z, newItem);
            }
        }
    }
    map->updateViews();
}
{
    // Store items that will be replaced during redo for undo
    QJsonArray tilesArray = clipboardData["tiles"].toArray();
    for (const QJsonValue& value : tilesArray) {
        QJsonObject tileObj = value.toObject();
        int relX = tileObj["x"].toInt();
        int relY = tileObj["y"].toInt();
        int z = tileObj["z"].toInt();

        QPoint targetTile(pastePos.x() + relX, pastePos.y() + relY);

        if (targetTile.x() >= 0 && targetTile.x() < map->getWidth() &&
            targetTile.y() >= 0 && targetTile.y() < map->getHeight() &&
            z >= 0 && z < 16)
        {
            Tile* tile = map->getTile(targetTile.x(), targetTile.y(), z);
            if (tile && !tile->getItems().isEmpty()) {
                replacedItems.append({targetTile, {z, tile->getItems()}});
            }
        }
    }
}
{
    // Clear the area where items were pasted
    QJsonArray tilesArray = clipboardData["tiles"].toArray();
    for (const QJsonValue& value : tilesArray) {
        QJsonObject tileObj = value.toObject();
        int relX = tileObj["x"].toInt();
        int relY = tileObj["y"].toInt();
        int z = tileObj["z"].toInt();

        QPoint targetTile(pastePos.x() + relX, pastePos.y() + relY);

        if (targetTile.x() >= 0 && targetTile.x() < map->getWidth() &&
            targetTile.y() >= 0 && targetTile.y() < map->getHeight() &&
            z >= 0 && z < 16)
        {
            map->clearItems(targetTile.x(), targetTile.y(), z);
        }
    }

    // Re-add the items that were replaced
    for (const auto& pair : replacedItems) {
        QPoint pos = pair.first;
        int layer = pair.second.first;
        const QList<Item>& items = pair.second.second;
        for (const Item& item : items) {
            map->addItem(pos.x(), pos.y(), layer, item);
        }
    }
    map->updateViews();
}
{
    // Clear the area where items will be pasted (this was done in undo, but redo needs to ensure it's clear)
    // Note: The constructor already stored replaced items, so we just need to clear and paste.
    QJsonArray tilesArray = clipboardData["tiles"].toArray();
    for (const QJsonValue& value : tilesArray) {
        QJsonObject tileObj = value.toObject();
        int relX = tileObj["x"].toInt();
        int relY = tileObj["y"].toInt();
        int z = tileObj["z"].toInt();

        QPoint targetTile(pastePos.x() + relX, pastePos.y() + relY);

        if (targetTile.x() >= 0 && targetTile.x() < map->getWidth() &&
            targetTile.y() >= 0 && targetTile.y() < map->getHeight() &&
            z >= 0 && z < 16)
        {
            map->clearItems(targetTile.x(), targetTile.y(), z);

            QJsonArray itemsArray = tileObj["items"].toArray();
            for (const QJsonValue& itemValue : itemsArray) {
                QJsonObject itemObj = itemValue.toObject();
                int id = itemObj["id"].toInt();
                int count = itemObj["count"].toInt();
                // TODO: Wczytaj inne właściwości przedmiotu jeśli zostały zapisane

                Item newItem(id, ""); // Use ItemManager::getItem() in the future
                newItem.setCount(count);

                map->addItem(targetTile.x(), targetTile.y(), z, newItem);
            }
        }
    }
    map->updateViews();
}
{
    // Store items that will be replaced during redo for undo
    QJsonArray tilesArray = clipboardData["tiles"].toArray();
    for (const QJsonValue& value : tilesArray) {
        QJsonObject tileObj = value.toObject();
        int relX = tileObj["x"].toInt();
        int relY = tileObj["y"].toInt();
        int z = tileObj["z"].toInt();

        QPoint targetTile(pastePos.x() + relX, pastePos.y() + relY);

        if (targetTile.x() >= 0 && targetTile.x() < map->getWidth() &&
            targetTile.y() >= 0 && targetTile.y() < map->getHeight() &&
            z >= 0 && z < 16)
        {
            Tile* tile = map->getTile(targetTile.x(), targetTile.y(), z);
            if (tile && !tile->getItems().isEmpty()) {
                replacedItems.append({targetTile, {z, tile->getItems()}});
            }
        }
    }
}
{
    // Clear the area where items were pasted
    QJsonArray tilesArray = clipboardData["tiles"].toArray();
    for (const QJsonValue& value : tilesArray) {
        QJsonObject tileObj = value.toObject();
        int relX = tileObj["x"].toInt();
        int relY = tileObj["y"].toInt();
        int z = tileObj["z"].toInt();

        QPoint targetTile(pastePos.x() + relX, pastePos.y() + relY);

        if (targetTile.x() >= 0 && targetTile.x() < map->getWidth() &&
            targetTile.y() >= 0 && targetTile.y() < map->getHeight() &&
            z >= 0 && z < 16)
        {
            map->clearItems(targetTile.x(), targetTile.y(), z);
        }
    }

    // Re-add the items that were replaced
    for (const auto& pair : replacedItems) {
        QPoint pos = pair.first;
        int layer = pair.second.first;
        const QList<Item>& items = pair.second.second;
        for (const Item& item : items) {
            map->addItem(pos.x(), pos.y(), layer, item);
        }
    }
    map->updateViews();
}
{
    // Clear the area where items will be pasted (this was done in undo, but redo needs to ensure it's clear)
    // Note: The constructor already stored replaced items, so we just need to clear and paste.
    QJsonArray tilesArray = clipboardData["tiles"].toArray();
    for (const QJsonValue& value : tilesArray) {
        QJsonObject tileObj = value.toObject();
        int relX = tileObj["x"].toInt();
        int relY = tileObj["y"].toInt();
        int z = tileObj["z"].toInt();

        QPoint targetTile(pastePos.x() + relX, pastePos.y() + relY);

        if (targetTile.x() >= 0 && targetTile.x() < map->getWidth() &&
            targetTile.y() >= 0 && targetTile.y() < map->getHeight() &&
            z >= 0 && z < 16)
        {
            map->clearItems(targetTile.x(), targetTile.y(), z);

            QJsonArray itemsArray = tileObj["items"].toArray();
            for (const QJsonValue& itemValue : itemsArray) {
                QJsonObject itemObj = itemValue.toObject();
                int id = itemObj["id"].toInt();
                int count = itemObj["count"].toInt();
                // TODO: Wczytaj inne właściwości przedmiotu jeśli zostały zapisane

                Item newItem(id, ""); // Use ItemManager::getItem() in the future
                newItem.setCount(count);

                map->addItem(targetTile.x(), targetTile.y(), z, newItem);
            }
        }
    }
    map->updateViews();
}
{
    // Store items that will be replaced during redo for undo
    QJsonArray tilesArray = clipboardData["tiles"].toArray();
    for (const QJsonValue& value : tilesArray) {
        QJsonObject tileObj = value.toObject();
        int relX = tileObj["x"].toInt();
        int relY = tileObj["y"].toInt();
        int z = tileObj["z"].toInt();

        QPoint targetTile(pastePos.x() + relX, pastePos.y() + relY);

        if (targetTile.x() >= 0 && targetTile.x() < map->getWidth() &&
            targetTile.y() >= 0 && targetTile.y() < map->getHeight() &&
            z >= 0 && z < 16)
        {
            Tile* tile = map->getTile(targetTile.x(), targetTile.y(), z);
            if (tile && !tile->getItems().isEmpty()) {
                replacedItems.append({targetTile, {z, tile->getItems()}});
            }
        }
    }
}
{
    // Clear the area where items were pasted
    QJsonArray tilesArray = clipboardData["tiles"].toArray();
    for (const QJsonValue& value : tilesArray) {
        QJsonObject tileObj = value.toObject();
        int relX = tileObj["x"].toInt();
        int relY = tileObj["y"].toInt();
        int z = tileObj["z"].toInt();

        QPoint targetTile(pastePos.x() + relX, pastePos.y() + relY);

        if (targetTile.x() >= 0 && targetTile.x() < map->getWidth() &&
            targetTile.y() >= 0 && targetTile.y() < map->getHeight() &&
            z >= 0 && z < 16)
        {
            map->clearItems(targetTile.x(), targetTile.y(), z);
        }
    }

    // Re-add the items that were replaced
    for (const auto& pair : replacedItems) {
        QPoint pos = pair.first;
        int layer = pair.second.first;
        const QList<Item>& items = pair.second.second;
        for (const Item& item : items) {
            map->addItem(pos.x(), pos.y(), layer, item);
        }
    }
    map->updateViews();
}
{
    // Clear the area where items will be pasted (this was done in undo, but redo needs to ensure it's clear)
    // Note: The constructor already stored replaced items, so we just need to clear and paste.
    QJsonArray tilesArray = clipboardData["tiles"].toArray();
    for (const QJsonValue& value : tilesArray) {
        QJsonObject tileObj = value.toObject();
        int relX = tileObj["x"].toInt();
        int relY = tileObj["y"].toInt();
        int z = tileObj["z"].toInt();

        QPoint targetTile(pastePos.x() + relX, pastePos.y() + relY);

        if (targetTile.x() >= 0 && targetTile.x() < map->getWidth() &&
            targetTile.y() >= 0 && targetTile.y() < map->getHeight() &&
            z >= 0 && z < 16)
        {
            map->clearItems(targetTile.x(), targetTile.y(), z);

            QJsonArray itemsArray = tileObj["items"].toArray();
            for (const QJsonValue& itemValue : itemsArray) {
                QJsonObject itemObj = itemValue.toObject();
                int id = itemObj["id"].toInt();
                int count = itemObj["count"].toInt();
                // TODO: Wczytaj inne właściwości przedmiotu jeśli zostały zapisane

                Item newItem(id, ""); // Use ItemManager::getItem() in the future
                newItem.setCount(count);

                map->addItem(targetTile.x(), targetTile.y(), z, newItem);
            }
        }
    }
    map->updateViews();
}
{
    // Store items that will be replaced during redo for undo
    QJsonArray tilesArray = clipboardData["tiles"].toArray();
    for (const QJsonValue& value : tilesArray) {
        QJsonObject tileObj = value.toObject();
        int relX = tileObj["x"].toInt();
        int relY = tileObj["y"].toInt();
        int z = tileObj["z"].toInt();

        QPoint targetTile(pastePos.x() + relX, pastePos.y() + relY);

        if (targetTile.x() >= 0 && targetTile.x() < map->getWidth() &&
            targetTile.y() >= 0 && targetTile.y() < map->getHeight() &&
            z >= 0 && z < 16)
        {
            Tile* tile = map->getTile(targetTile.x(), targetTile.y(), z);
            if (tile && !tile->getItems().isEmpty()) {
                replacedItems.append({targetTile, {z, tile->getItems()}});
            }
        }
    }
}
{
    // Clear the area where items were pasted
    QJsonArray tilesArray = clipboardData["tiles"].toArray();
    for (const QJsonValue& value : tilesArray) {
        QJsonObject tileObj = value.toObject();
        int relX = tileObj["x"].toInt();
        int relY = tileObj["y"].toInt();
        int z = tileObj["z"].toInt();

        QPoint targetTile(pastePos.x() + relX, pastePos.y() + relY);

        if (targetTile.x() >= 0 && targetTile.x() < map->getWidth() &&
            targetTile.y() >= 0 && targetTile.y() < map->getHeight() &&
            z >= 0 && z < 16)
        {
            map->clearItems(targetTile.x(), targetTile.y(), z);
        }
    }

    // Re-add the items that were replaced
    for (const auto& pair : replacedItems) {
        QPoint pos = pair.first;
        int layer = pair.second.first;
        const QList<Item>& items = pair.second.second;
        for (const Item& item : items) {
            map->addItem(pos.x(), pos.y(), layer, item);
        }
    }
    map->updateViews();
}
{
    // Clear the area where items will be pasted (this was done in undo, but redo needs to ensure it's clear)
    // Note: The constructor already stored replaced items, so we just need to clear and paste.
    QJsonArray tilesArray = clipboardData["tiles"].toArray();
    for (const QJsonValue& value : tilesArray) {
        QJsonObject tileObj = value.toObject();
        int relX = tileObj["x"].toInt();
        int relY = tileObj["y"].toInt();
        int z = tileObj["z"].toInt();

        QPoint targetTile(pastePos.x() + relX, pastePos.y() + relY);

        if (targetTile.x() >= 0 && targetTile.x() < map->getWidth() &&
            targetTile.y() >= 0 && targetTile.y() < map->getHeight() &&
            z >= 0 && z < 16)
        {
            map->clearItems(targetTile.x(), targetTile.y(), z);

            QJsonArray itemsArray = tileObj["items"].toArray();
            for (const QJsonValue& itemValue : itemsArray) {
                QJsonObject itemObj = itemValue.toObject();
                int id = itemObj["id"].toInt();
                int count = itemObj["count"].toInt();
                // TODO: Wczytaj inne właściwości przedmiotu jeśli zostały zapisane

                Item newItem(id, ""); // Use ItemManager::getItem() in the future
                newItem.setCount(count);

                map->addItem(targetTile.x(), targetTile.y(), z, newItem);
            }
        }
    }
    map->updateViews();
}
{
    // Store items that will be replaced during redo for undo
    QJsonArray tilesArray = clipboardData["tiles"].toArray();
    for (const QJsonValue& value : tilesArray) {
        QJsonObject tileObj = value.toObject();
        int relX = tileObj["x"].toInt();
        int relY = tileObj["y"].toInt();
        int z = tileObj["z"].toInt();

        QPoint targetTile(pastePos.x() + relX, pastePos.y() + relY);

        if (targetTile.x() >= 0 && targetTile.x() < map->getWidth() &&
            targetTile.y() >= 0 && targetTile.y() < map->getHeight() &&
            z >= 0 && z < 16)
        {
            Tile* tile = map->getTile(targetTile.x(), targetTile.y(), z);
            if (tile && !tile->getItems().isEmpty()) {
                replacedItems.append({targetTile, {z, tile->getItems()}});
            }
        }
    }
}
{
    // Clear the area where items were pasted
    QJsonArray tilesArray = clipboardData["tiles"].toArray();
    for (const QJsonValue& value : tilesArray) {
        QJsonObject tileObj = value.toObject();
        int relX = tileObj["x"].toInt();
        int relY = tileObj["y"].toInt();
        int z = tileObj["z"].toInt();

        QPoint targetTile(pastePos.x() + relX, pastePos.y() + relY);

        if (targetTile.x() >= 0 && targetTile.x() < map->getWidth() &&
            targetTile.y() >= 0 && targetTile.y() < map->getHeight() &&
            z >= 0 && z < 16)
        {
            map->clearItems(targetTile.x(), targetTile.y(), z);
        }
    }

    // Re-add the items that were replaced
    for (const auto& pair : replacedItems) {
        QPoint pos = pair.first;
        int layer = pair.second.first;
        const QList<Item>& items = pair.second.second;
        for (const Item& item : items) {
            map->addItem(pos.x(), pos.y(), layer, item);
        }
    }
    map->updateViews();
}
{
    // Clear the area where items will be pasted (this was done in undo, but redo needs to ensure it's clear)
    // Note: The constructor already stored replaced items, so we just need to clear and paste.
    QJsonArray tilesArray = clipboardData["tiles"].toArray();
    for (const QJsonValue& value : tilesArray) {
        QJsonObject tileObj = value.toObject();
        int relX = tileObj["x"].toInt();
        int relY = tileObj["y"].toInt();
        int z = tileObj["z"].toInt();

        QPoint targetTile(pastePos.x() + relX, pastePos.y() + relY);

        if (targetTile.x() >= 0 && targetTile.x() < map->getWidth() &&
            targetTile.y() >= 0 && targetTile.y() < map->getHeight() &&
            z >= 0 && z < 16)
        {
            map->clearItems(targetTile.x(), targetTile.y(), z);

            QJsonArray itemsArray = tileObj["items"].toArray();
            for (const QJsonValue& itemValue : itemsArray) {
                QJsonObject itemObj = itemValue.toObject();
                int id = itemObj["id"].toInt();
                int count = itemObj["count"].toInt();
                // TODO: Wczytaj inne właściwości przedmiotu jeśli zostały zapisane

                Item newItem(id, ""); // Use ItemManager::getItem() in the future
                newItem.setCount(count);

                map->addItem(targetTile.x(), targetTile.y(), z, newItem);
            }
        }
    }
    map->updateViews();
}
{
    // Store items that will be replaced during redo for undo
    QJsonArray tilesArray = clipboardData["tiles"].toArray();
    for (const QJsonValue& value : tilesArray) {
        QJsonObject tileObj = value.toObject();
        int relX = tileObj["x"].toInt();
        int relY = tileObj["y"].toInt();
        int z = tileObj["z"].toInt();

        QPoint targetTile(pastePos.x() + relX, pastePos.y() + relY);

        if (targetTile.x() >= 0 && targetTile.x() < map->getWidth() &&
            targetTile.y() >= 0 && targetTile.y() < map->getHeight() &&
            z >= 0 && z < 16)
        {
            Tile* tile = map->getTile(targetTile.x(), targetTile.y(), z);
            if (tile && !tile->getItems().isEmpty()) {
                replacedItems.append({targetTile, {z, tile->getItems()}});
            }
        }
    }
}
{
    // Clear the area where items were pasted
    QJsonArray tilesArray = clipboardData["tiles"].toArray();
    for (const QJsonValue& value : tilesArray) {
        QJsonObject tileObj = value.toObject();
        int relX = tileObj["x"].toInt();
        int relY = tileObj["y"].toInt();
        int z = tileObj["z"].toInt();

        QPoint targetTile(pastePos.x() + relX, pastePos.y() + relY);

        if (targetTile.x() >= 0 && targetTile.x() < map->getWidth() &&
            targetTile.y() >= 0 && targetTile.y() < map->getHeight() &&
            z >= 0 && z < 16)
        {
            map->clearItems(targetTile.x(), targetTile.y(), z);
        }
    }

    // Re-add the items that were replaced
    for (const auto& pair : replacedItems) {
        QPoint pos = pair.first;
        int layer = pair.second.first;
        const QList<Item>& items = pair.second.second;
        for (const Item& item : items) {
            map->addItem(pos.x(), pos.y(), layer, item);
        }
    }
    map->updateViews();
}
{
    // Clear the area where items will be pasted (this was done in undo, but redo needs to ensure it's clear)
    // Note: The constructor already stored replaced items, so we just need to clear and paste.
    QJsonArray tilesArray = clipboardData["tiles"].toArray();
    for (const QJsonValue& value : tilesArray) {
        QJsonObject tileObj = value.toObject();
        int relX = tileObj["x"].toInt();
        int relY = tileObj["y"].toInt();
        int z = tileObj["z"].toInt();

        QPoint targetTile(pastePos.x() + relX, pastePos.y() + relY);

        if (targetTile.x() >= 0 && targetTile.x() < map->getWidth() &&
            targetTile.y() >= 0 && targetTile.y() < map->getHeight() &&
            z >= 0 && z < 16)
        {
            map->clearItems(targetTile.x(), targetTile.y(), z);

            QJsonArray itemsArray = tileObj["items"].toArray();
            for (const QJsonValue& itemValue : itemsArray) {
                QJsonObject itemObj = itemValue.toObject();
                int id = itemObj["id"].toInt();
                int count = itemObj["count"].toInt();
                // TODO: Wczytaj inne właściwości przedmiotu jeśli zostały zapisane

                Item newItem(id, ""); // Use ItemManager::getItem() in the future
                newItem.setCount(count);

                map->addItem(targetTile.x(), targetTile.y(), z, newItem);
            }
        }
    }
    map->updateViews();
}
{
    // Store items that will be replaced during redo for undo
    QJsonArray tilesArray = clipboardData["tiles"].toArray();
    for (const QJsonValue& value : tilesArray) {
        QJsonObject tileObj = value.toObject();
        int relX = tileObj["x"].toInt();
        int relY = tileObj["y"].toInt();
        int z = tileObj["z"].toInt();

        QPoint targetTile(pastePos.x() + relX, pastePos.y() + relY);

        if (targetTile.x() >= 0 && targetTile.x() < map->getWidth() &&
            targetTile.y() >= 0 && targetTile.y() < map->getHeight() &&
            z >= 0 && z < 16)
        {
            Tile* tile = map->getTile(targetTile.x(), targetTile.y(), z);
            if (tile && !tile->getItems().isEmpty()) {
                replacedItems.append({targetTile, {z, tile->getItems()}});
            }
        }
    }
}
{
    // Clear the area where items were pasted
    QJsonArray tilesArray = clipboardData["tiles"].toArray();
    for (const QJsonValue& value : tilesArray) {
        QJsonObject tileObj = value.toObject();
        int relX = tileObj["x"].toInt();
        int relY = tileObj["y"].toInt();
        int z = tileObj["z"].toInt();

        QPoint targetTile(pastePos.x() + relX, pastePos.y() + relY);

        if (targetTile.x() >= 0 && targetTile.x() < map->getWidth() &&
            targetTile.y() >= 0 && targetTile.y() < map->getHeight() &&
            z >= 0 && z < 16)
        {
            map->clearItems(targetTile.x(), targetTile.y(), z);
        }
    }

    // Re-add the items that were replaced
    for (const auto& pair : replacedItems) {
        QPoint pos = pair.first;
        int layer = pair.second.first;
        const QList<Item>& items = pair.second.second;
        for (const Item& item : items) {
            map->addItem(pos.x(), pos.y(), layer, item);
        }
    }
    map->updateViews();
}
{
    // Clear the area where items will be pasted (this was done in undo, but redo needs to ensure it's clear)
    // Note: The constructor already stored replaced items, so we just need to clear and paste.
    QJsonArray tilesArray = clipboardData["tiles"].toArray();
    for (const QJsonValue& value : tilesArray) {
        QJsonObject tileObj = value.toObject();
        int relX = tileObj["x"].toInt();
        int relY = tileObj["y"].toInt();
        int z = tileObj["z"].toInt();

        QPoint targetTile(pastePos.x() + relX, pastePos.y() + relY);

        if (targetTile.x() >= 0 && targetTile.x() < map->getWidth() &&
            targetTile.y() >= 0 && targetTile.y() < map->getHeight() &&
            z >= 0 && z < 16)
        {
            map->clearItems(targetTile.x(), targetTile.y(), z);

            QJsonArray itemsArray = tileObj["items"].toArray();
            for (const QJsonValue& itemValue : itemsArray) {
                QJsonObject itemObj = itemValue.toObject();
                int id = itemObj["id"].toInt();
                int count = itemObj["count"].toInt();
                // TODO: Wczytaj inne właściwości przedmiotu jeśli zostały zapisane

                Item newItem(id, ""); // Use ItemManager::getItem() in the future
                newItem.setCount(count);

                map->addItem(targetTile.x(), targetTile.y(), z, newItem);
            }
        }
    }
    map->updateViews();
}
{
    // Store items that will be replaced during redo for undo
    QJsonArray tilesArray = clipboardData["tiles"].toArray();
    for (const QJsonValue& value : tilesArray) {
        QJsonObject tileObj = value.toObject();
        int relX = tileObj["x"].toInt();
        int relY = tileObj["y"].toInt();
        int z = tileObj["z"].toInt();

        QPoint targetTile(pastePos.x() + relX, pastePos.y() + relY);

        if (targetTile.x() >= 0 && targetTile.x() < map->getWidth() &&
            targetTile.y() >= 0 && targetTile.y() < map->getHeight() &&
            z >= 0 && z < 16)
        {
            Tile* tile = map->getTile(targetTile.x(), targetTile.y(), z);
            if (tile && !tile->getItems().isEmpty()) {
                replacedItems.append({targetTile, {z, tile->getItems()}});
            }
        }
    }
}
{
    // Clear the area where items were pasted
    QJsonArray tilesArray = clipboardData["tiles"].toArray();
    for (const QJsonValue& value : tilesArray) {
        QJsonObject tileObj = value.toObject();
        int relX = tileObj["x"].toInt();
        int relY = tileObj["y"].toInt();
        int z = tileObj["z"].toInt();

        QPoint targetTile(pastePos.x() + relX, pastePos.y() + relY);

        if (targetTile.x() >= 0 && targetTile.x() < map->getWidth() &&
            targetTile.y() >= 0 && targetTile.y() < map->getHeight() &&
            z >= 0 && z < 16)
        {
            map->clearItems(targetTile.x(), targetTile.y(), z);
        }
    }

    // Re-add the items that were replaced
    for (const auto& pair : replacedItems) {
        QPoint pos = pair.first;
        int layer = pair.second.first;
        const QList<Item>& items = pair.second.second;
        for (const Item& item : items) {
            map->addItem(pos.x(), pos.y(), layer, item);
        }
    }
    map->updateViews();
}
{
    // Clear the area where items will be pasted (this was done in undo, but redo needs to ensure it's clear)
    // Note: The constructor already stored replaced items, so we just need to clear and paste.
    QJsonArray tilesArray = clipboardData["tiles"].toArray();
    for (const QJsonValue& value : tilesArray) {
        QJsonObject tileObj = value.toObject();
        int relX = tileObj["x"].toInt();
        int relY = tileObj["y"].toInt();
        int z = tileObj["z"].toInt();

        QPoint targetTile(pastePos.x() + relX, pastePos.y() + relY);

        if (targetTile.x() >= 0 && targetTile.x() < map->getWidth() &&
            targetTile.y() >= 0 && targetTile.y() < map->getHeight() &&
            z >= 0 && z < 16)
        {
            map->clearItems(targetTile.x(), targetTile.y(), z);

            QJsonArray itemsArray = tileObj["items"].toArray();
            for (const QJsonValue& itemValue : itemsArray) {
                QJsonObject itemObj = itemValue.toObject();
                int id = itemObj["id"].toInt();
                int count = itemObj["count"].toInt();
                // TODO: Wczytaj inne właściwości przedmiotu jeśli zostały zapisane

                Item newItem(id, ""); // Use ItemManager::getItem() in the future
                newItem.setCount(count);

                map->addItem(targetTile.x(), targetTile.y(), z, newItem);
            }
        }
    }
    map->updateViews();
}
{
    // Store items that will be replaced during redo for undo
    QJsonArray tilesArray = clipboardData["tiles"].toArray();
    for (const QJsonValue& value : tilesArray) {
        QJsonObject tileObj = value.toObject();
        int relX = tileObj["x"].toInt();
        int relY = tileObj["y"].toInt();
        int z = tileObj["z"].toInt();

        QPoint targetTile(pastePos.x() + relX, pastePos.y() + relY);

        if (targetTile.x() >= 0 && targetTile.x() < map->getWidth() &&
            targetTile.y() >= 0 && targetTile.y() < map->getHeight() &&
            z >= 0 && z < 16)
        {
            Tile* tile = map->getTile(targetTile.x(), targetTile.y(), z);
            if (tile && !tile->getItems().isEmpty()) {
                replacedItems.append({targetTile, {z, tile->getItems()}});
            }
        }
    }
}
{
    // Clear the area where items were pasted
    QJsonArray tilesArray = clipboardData["tiles"].toArray();
    for (const QJsonValue& value : tilesArray) {
        QJsonObject tileObj = value.toObject();
        int relX = tileObj["x"].toInt();
        int relY = tileObj["y"].toInt();
        int z = tileObj["z"].toInt();

        QPoint targetTile(pastePos.x() + relX, pastePos.y() + relY);

        if (targetTile.x() >= 0 && targetTile.x() < map->getWidth() &&
            targetTile.y() >= 0 && targetTile.y() < map->getHeight() &&
            z >= 0 && z < 16)
        {
            map->clearItems(targetTile.x(), targetTile.y(), z);
        }
    }

    // Re-add the items that were replaced
    for (const auto& pair : replacedItems) {
        QPoint pos = pair.first;
        int layer = pair.second.first;
        const QList<Item>& items = pair.second.second;
        for (const Item& item : items) {
            map->addItem(pos.x(), pos.y(), layer, item);
        }
    }
    map->updateViews();
}
{
    // Clear the area where items will be pasted (this was done in undo, but redo needs to ensure it's clear)
    // Note: The constructor already stored replaced items, so we just need to clear and paste.
    QJsonArray tilesArray = clipboardData["tiles"].toArray();
    for (const QJsonValue& value : tilesArray) {
        QJsonObject tileObj = value.toObject();
        int relX = tileObj["x"].toInt();
        int relY = tileObj["y"].toInt();
        int z = tileObj["z"].toInt();

        QPoint targetTile(pastePos.x() + relX, pastePos.y() + relY);

        if (targetTile.x() >= 0 && targetTile.x() < map->getWidth() &&
            targetTile.y() >= 0 && targetTile.y() < map->getHeight() &&
            z >= 0 && z < 16)
        {
            map->clearItems(targetTile.x(), targetTile.y(), z);

            QJsonArray itemsArray = tileObj["items"].toArray();
            for (const QJsonValue& itemValue : itemsArray) {
                QJsonObject itemObj = itemValue.toObject();
                int id = itemObj["id"].toInt();
                int count = itemObj["count"].toInt();
                // TODO: Wczytaj inne właściwości przedmiotu jeśli zostały zapisane

                Item newItem(id, ""); // Use ItemManager::getItem() in the future
                newItem.setCount(count);

                map->addItem(targetTile.x(), targetTile.y(), z, newItem);
            }
        }
    }
    map->updateViews();
}
{
    // Store items that will be replaced during redo for undo
    QJsonArray tilesArray = clipboardData["tiles"].toArray();
    for (const QJsonValue& value : tilesArray) {
        QJsonObject tileObj = value.toObject();
        int relX = tileObj["x"].toInt();
        int relY = tileObj["y"].toInt();
        int z = tileObj["z"].toInt();

        QPoint targetTile(pastePos.x() + relX, pastePos.y() + relY);

        if (targetTile.x() >= 0 && targetTile.x() < map->getWidth() &&
            targetTile.y() >= 0 && targetTile.y() < map->getHeight() &&
            z >= 0 && z < 16)
        {
            Tile* tile = map->getTile(targetTile.x(), targetTile.y(), z);
            if (tile && !tile->getItems().isEmpty()) {
                replacedItems.append({targetTile, {z, tile->getItems()}});
            }
        }
    }
}
{
    // Clear the area where items were pasted
    QJsonArray tilesArray = clipboardData["tiles"].toArray();
    for (const QJsonValue& value : tilesArray) {
        QJsonObject tileObj = value.toObject();
        int relX = tileObj["x"].toInt();
        int relY = tileObj["y"].toInt();
        int z = tileObj["z"].toInt();

        QPoint targetTile(pastePos.x() + relX, pastePos.y() + relY);

        if (targetTile.x() >= 0 && targetTile.x() < map->getWidth() &&
            targetTile.y() >= 0 && targetTile.y() < map->getHeight() &&
            z >= 0 && z < 16)
        {
            map->clearItems(targetTile.x(), targetTile.y(), z);
        }
    }

    // Re-add the items that were replaced
    for (const auto& pair : replacedItems) {
        QPoint pos = pair.first;
        int layer = pair.second.first;
        const QList<Item>& items = pair.second.second;
        for (const Item& item : items) {
            map->addItem(pos.x(), pos.y(), layer, item);
        }
    }
    map->updateViews();
}
{
    // Clear the area where items will be pasted (this was done in undo, but redo needs to ensure it's clear)
    // Note: The constructor already stored replaced items, so we just need to clear and paste.
    QJsonArray tilesArray = clipboardData["tiles"].toArray();
    for (const QJsonValue& value : tilesArray) {
        QJsonObject tileObj = value.toObject();
        int relX = tileObj["x"].toInt();
        int relY = tileObj["y"].toInt();
        int z = tileObj["z"].toInt();

        QPoint targetTile(pastePos.x() + relX, pastePos.y() + relY);

        if (targetTile.x() >= 0 && targetTile.x() < map->getWidth() &&
            targetTile.y() >= 0 && targetTile.y() < map->getHeight() &&
            z >= 0 && z < 16)
        {
            map->clearItems(targetTile.x(), targetTile.y(), z);

            QJsonArray itemsArray = tileObj["items"].toArray();
            for (const QJsonValue& itemValue : itemsArray) {
                QJsonObject itemObj = itemValue.toObject();
                int id = itemObj["id"].toInt();
                int count = itemObj["count"].toInt();
                // TODO: Wczytaj inne właściwości przedmiotu jeśli zostały zapisane

                Item newItem(id, ""); // Use ItemManager::getItem() in the future
                newItem.setCount(count);

                map->addItem(targetTile.x(), targetTile.y(), z, newItem);
            }
        }
    }
    map->updateViews();
}
{
    // Store items that will be replaced during redo for undo
    QJsonArray tilesArray = clipboardData["tiles"].toArray();
    for (const QJsonValue& value : tilesArray) {
        QJsonObject tileObj = value.toObject();
        int relX = tileObj["x"].toInt();
        int relY = tileObj["y"].toInt();
        int z = tileObj["z"].toInt();

        QPoint targetTile(pastePos.x() + relX, pastePos.y() + relY);

        if (targetTile.x() >= 0 && targetTile.x() < map->getWidth() &&
            targetTile.y() >= 0 && targetTile.y() < map->getHeight() &&
            z >= 0 && z < 16)
        {
            Tile* tile = map->getTile(targetTile.x(), targetTile.y(), z);
            if (tile && !tile->getItems().isEmpty()) {
                replacedItems.append({targetTile, {z, tile->getItems()}});
            }
        }
    }
}
{
    // Clear the area where items were pasted
    QJsonArray tilesArray = clipboardData["tiles"].toArray();
    for (const QJsonValue& value : tilesArray) {
        QJsonObject tileObj = value.toObject();
        int relX = tileObj["x"].toInt();
        int relY = tileObj["y"].toInt();
        int z = tileObj["z"].toInt();

        QPoint targetTile(pastePos.x() + relX, pastePos.y() + relY);

        if (targetTile.x() >= 0 && targetTile.x() < map->getWidth() &&
            targetTile.y() >= 0 && targetTile.y() < map->getHeight() &&
            z >= 0 && z < 16)
        {
            map->clearItems(targetTile.x(), targetTile.y(), z);
        }
    }

    // Re-add the items that were replaced
    for (const auto& pair : replacedItems) {
        QPoint pos = pair.first;
        int layer = pair.second.first;
        const QList<Item>& items = pair.second.second;
        for (const Item& item : items) {
            map->addItem(pos.x(), pos.y(), layer, item);
        }
    }
    map->updateViews();
}
{
    // Clear the area where items will be pasted (this was done in undo, but redo needs to ensure it's clear)
    // Note: The constructor already stored replaced items, so we just need to clear and paste.
    QJsonArray tilesArray = clipboardData["tiles"].toArray();
    for (const QJsonValue& value : tilesArray) {
        QJsonObject tileObj = value.toObject();
        int relX = tileObj["x"].toInt();
        int relY = tileObj["y"].toInt();
        int z = tileObj["z"].toInt();

        QPoint targetTile(pastePos.x() + relX, pastePos.y() + relY);

        if (targetTile.x() >= 0 && targetTile.x() < map->getWidth() &&
            targetTile.y() >= 0 && targetTile.y() < map->getHeight() &&
            z >= 0 && z < 16)
        {
            map->clearItems(targetTile.x(), targetTile.y(), z);

            QJsonArray itemsArray = tileObj["items"].toArray();
            for (const QJsonValue& itemValue : itemsArray) {
                QJsonObject itemObj = itemValue.toObject();
                int id = itemObj["id"].toInt();
                int count = itemObj["count"].toInt();
                // TODO: Wczytaj inne właściwości przedmiotu jeśli zostały zapisane

                Item newItem(id, ""); // Use ItemManager::getItem() in the future
                newItem.setCount(count);

                map->addItem(targetTile.x(), targetTile.y(), z, newItem);
            }
        }
    }
    map->updateViews();
}
{
    // Store items that will be replaced during redo for undo
    QJsonArray tilesArray = clipboardData["tiles"].toArray();
    for (const QJsonValue& value : tilesArray) {
        QJsonObject tileObj = value.toObject();
        int relX = tileObj["x"].toInt();
        int relY = tileObj["y"].toInt();
        int z = tileObj["z"].toInt();

        QPoint targetTile(pastePos.x() + relX, pastePos.y() + relY);

        if (targetTile.x() >= 0 && targetTile.x() < map->getWidth() &&
            targetTile.y() >= 0 && targetTile.y() < map->getHeight() &&
            z >= 0 && z < 16)
        {
            Tile* tile = map->getTile(targetTile.x(), targetTile.y(), z);
            if (tile && !tile->getItems().isEmpty()) {
                replacedItems.append({targetTile, {z, tile->getItems()}});
            }
        }
    }
}
{
    // Clear the area where items were pasted
    QJsonArray tilesArray = clipboardData["tiles"].toArray();
    for (const QJsonValue& value : tilesArray) {
        QJsonObject tileObj = value.toObject();
        int relX = tileObj["x"].toInt();
        int relY = tileObj["y"].toInt();
        int z = tileObj["z"].toInt();

        QPoint targetTile(pastePos.x() + relX, pastePos.y() + relY);

        if (targetTile.x() >= 0 && targetTile.x() < map->getWidth() &&
            targetTile.y() >= 0 && targetTile.y() < map->getHeight() &&
            z >= 0 && z < 16)
        {
            map->clearItems(targetTile.x(), targetTile.y(), z);
        }
    }

    // Re-add the items that were replaced
    for (const auto& pair : replacedItems) {
        QPoint pos = pair.first;
        int layer = pair.second.first;
        const QList<Item>& items = pair.second.second;
        for (const Item& item : items) {
            map->addItem(pos.x(), pos.y(), layer, item);
        }
    }
    map->updateViews();
}
{
    // Clear the area where items will be pasted (this was done in undo, but redo needs to ensure it's clear)
    // Note: The constructor already stored replaced items, so we just need to clear and paste.
    QJsonArray tilesArray = clipboardData["tiles"].toArray();
    for (const QJsonValue& value : tilesArray) {
        QJsonObject tileObj = value.toObject();
        int relX = tileObj["x"].toInt();
        int relY = tileObj["y"].toInt();
        int z = tileObj["z"].toInt();

        QPoint targetTile(pastePos.x() + relX, pastePos.y() + relY);

        if (targetTile.x() >= 0 && targetTile.x() < map->getWidth() &&
            targetTile.y() >= 0 && targetTile.y() < map->getHeight() &&
            z >= 0 && z < 16)
        {
            map->clearItems(targetTile.x(), targetTile.y(), z);

            QJsonArray itemsArray = tileObj["items"].toArray();
            for (const QJsonValue& itemValue : itemsArray) {
                QJsonObject itemObj = itemValue.toObject();
                int id = itemObj["id"].toInt();
                int count = itemObj["count"].toInt();
                // TODO: Wczytaj inne właściwości przedmiotu jeśli zostały zapisane

                Item newItem(id, ""); // Use ItemManager::getItem() in the future
                newItem.setCount(count);

                map->addItem(targetTile.x(), targetTile.y(), z, newItem);
            }
        }
    }
    map->updateViews();
}
{
    // Store items that will be replaced during redo for undo
    QJsonArray tilesArray = clipboardData["tiles"].toArray();
    for (const QJsonValue& value : tilesArray) {
        QJsonObject tileObj = value.toObject();
        int relX = tileObj["x"].toInt();
        int relY = tileObj["y"].toInt();
        int z = tileObj["z"].toInt();

        QPoint targetTile(pastePos.x() + relX, pastePos.y() + relY);

        if (targetTile.x() >= 0 && targetTile.x() < map->getWidth() &&
            targetTile.y() >= 0 && targetTile.y() < map->getHeight() &&
            z >= 0 && z < 16)
        {
            Tile* tile = map->getTile(targetTile.x(), targetTile.y(), z);
            if (tile && !tile->getItems().isEmpty()) {
                replacedItems.append({targetTile, {z, tile->getItems()}});
            }
        }
    }
}
{
    // Clear the area where items were pasted
    QJsonArray tilesArray = clipboardData["tiles"].toArray();
    for (const QJsonValue& value : tilesArray) {
        QJsonObject tileObj = value.toObject();
        int relX = tileObj["x"].toInt();
        int relY = tileObj["y"].toInt();
        int z = tileObj["z"].toInt();

        QPoint targetTile(pastePos.x() + relX, pastePos.y() + relY);

        if (targetTile.x() >= 0 && targetTile.x() < map->getWidth() &&
            targetTile.y() >= 0 && targetTile.y() < map->getHeight() &&
            z >= 0 && z < 16)
        {
            map->clearItems(targetTile.x(), targetTile.y(), z);
        }
    }

    // Re-add the items that were replaced
    for (const auto& pair : replacedItems) {
        QPoint pos = pair.first;
        int layer = pair.second.first;
        const QList<Item>& items = pair.second.second;
        for (const Item& item : items) {
            map->addItem(pos.x(), pos.y(), layer, item);
        }
    }
    map->updateViews();
}
{
    // Clear the area where items will be pasted (this was done in undo, but redo needs to ensure it's clear)
    // Note: The constructor already stored replaced items, so we just need to clear and paste.
    QJsonArray tilesArray = clipboardData["tiles"].toArray();
    for (const QJsonValue& value : tilesArray) {
        QJsonObject tileObj = value.toObject();
        int relX = tileObj["x"].toInt();
        int relY = tileObj["y"].toInt();
        int z = tileObj["z"].toInt();

        QPoint targetTile(pastePos.x() + relX, pastePos.y() + relY);

        if (targetTile.x() >= 0 && targetTile.x() < map->getWidth() &&
            targetTile.y() >= 0 && targetTile.y() < map->getHeight() &&
            z >= 0 && z < 16)
        {
            map->clearItems(targetTile.x(), targetTile.y(), z);

            QJsonArray itemsArray = tileObj["items"].toArray();
            for (const QJsonValue& itemValue : itemsArray) {
                QJsonObject itemObj = itemValue.toObject();
                int id = itemObj["id"].toInt();
                int count = itemObj["count"].toInt();
                // TODO: Wczytaj inne właściwości przedmiotu jeśli zostały zapisane

                Item newItem(id, ""); // Use ItemManager::getItem() in the future
                newItem.setCount(count);

                map->addItem(targetTile.x(), targetTile.y(), z, newItem);
            }
        }
    }
    map->updateViews();
}
{
    // Store items that will be replaced during redo for undo
    QJsonArray tilesArray = clipboardData["tiles"].toArray();
    for (const QJsonValue& value : tilesArray) {
        QJsonObject tileObj = value.toObject();
        int relX = tileObj["x"].toInt();
        int relY = tileObj["y"].toInt();
        int z = tileObj["z"].toInt();

        QPoint targetTile(pastePos.x() + relX, pastePos.y() + relY);

        if (targetTile.x() >= 0 && targetTile.x() < map->getWidth() &&
            targetTile.y() >= 0 && targetTile.y() < map->getHeight() &&
            z >= 0 && z < 16)
        {
            Tile* tile = map->getTile(targetTile.x(), targetTile.y(), z);
            if (tile && !tile->getItems().isEmpty()) {
                replacedItems.append({targetTile, {z, tile->getItems()}});
            }
        }
    }
}
{
    // Clear the area where items were pasted
    QJsonArray tilesArray = clipboardData["tiles"].toArray();
    for (const QJsonValue& value : tilesArray) {
        QJsonObject tileObj = value.toObject();
        int relX = tileObj["x"].toInt();
        int relY = tileObj["y"].toInt();
        int z = tileObj["z"].toInt();

        QPoint targetTile(pastePos.x() + relX, pastePos.y() + relY);

        if (targetTile.x() >= 0 && targetTile.x() < map->getWidth() &&
            targetTile.y() >= 0 && targetTile.y() < map->getHeight() &&
            z >= 0 && z < 16)
        {
            map->clearItems(targetTile.x(), targetTile.y(), z);
        }
    }

    // Re-add the items that were replaced
    for (const auto& pair : replacedItems) {
        QPoint pos = pair.first;
        int layer = pair.second.first;
        const QList<Item>& items = pair.second.second;
        for (const Item& item : items) {
            map->addItem(pos.x(), pos.y(), layer, item);
        }
    }
    map->updateViews();
}
{
    // Clear the area where items will be pasted (this was done in undo, but redo needs to ensure it's clear)
    // Note: The constructor already stored replaced items, so we just need to clear and paste.
    QJsonArray tilesArray = clipboardData["tiles"].toArray();
    for (const QJsonValue& value : tilesArray) {
        QJsonObject tileObj = value.toObject();
        int relX = tileObj["x"].toInt();
        int relY = tileObj["y"].toInt();
        int z = tileObj["z"].toInt();

        QPoint targetTile(pastePos.x() + relX, pastePos.y() + relY);

        if (targetTile.x() >= 0 && targetTile.x() < map->getWidth() &&
            targetTile.y() >= 0 && targetTile.y() < map->getHeight() &&
            z >= 0 && z < 16)
        {
            map->clearItems(targetTile.x(), targetTile.y(), z);

            QJsonArray itemsArray = tileObj["items"].toArray();
            for (const QJsonValue& itemValue : itemsArray) {
                QJsonObject itemObj = itemValue.toObject();
                int id = itemObj["id"].toInt();
                int count = itemObj["count"].toInt();
                // TODO: Wczytaj inne właściwości przedmiotu jeśli zostały zapisane

                Item newItem(id, ""); // Use ItemManager::getItem() in the future
                newItem.setCount(count);

                map->addItem(targetTile.x(), targetTile.y(), z, newItem);
            }
        }
    }
    map->updateViews();
}
{
    // Store items that will be replaced during redo for undo
    QJsonArray tilesArray = clipboardData["tiles"].toArray();