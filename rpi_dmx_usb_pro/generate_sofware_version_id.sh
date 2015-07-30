echo "// Generated "$(date) > ./include/sofware_version_id.h
var="$(date +%s)"
echo "static const uint32_t DEVICE_SOFTWARE_VERSION_ID="$var";" >> ./include/sofware_version_id.h
