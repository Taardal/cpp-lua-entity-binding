require("player")
require("camera")
require("debug")

local entities = {}

function onCreateEntity(entity)
    entities[entity.entityId] = entity
    if (entity.onCreate ~= nil) then
        entity:onCreate()
    end
end

function onUpdateEntity(entityId)
    entities[entityId]:onUpdate()
end

function onDestroyEntity(entityId)
    entity = entities[entityId]
    if (entity.onDestroy ~= nil) then
        entity:onDestroy()
    end
    entities[entityId] = nil
end