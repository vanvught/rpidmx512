echo "// Generated "$(date) > sofware_version_id.h
var="$(date +%s)"
echo "constexpr uint32_t DEVICE_SOFTWARE_VERSION_ID="$var";" >> sofware_version_id.h

