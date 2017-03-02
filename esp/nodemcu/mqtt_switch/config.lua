-- file : config.lua
local module = {}

module.SSID = {}
module.SSID["dd-wrt-low"] = "sabarasa"

module.HOST = "192.168.0.105"
module.PORT = 1883
module.ID = node.chipid()

module.ENDPOINT = "esp/"
return module