-- file : config.lua
local module = {}

module.SSID = {}
module.SSID["dd-wrt-low"] = "sabarasa"

module.HOST = "192.168.0.105"
module.PORT = 1883
module.ID = node.chipid()

module.GPIO0 = 3
module.GPIO2 = 4

module.ENDPOINT = "esp"
module.MAX_SUBS_RETRY = 15
module.SUBS_RETRY_DELAY = 1000

return module
