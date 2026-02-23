/*
 * emac_counters.cpp
 */
 
#include "linux/network.h"

#if defined(__APPLE__)
#include <sys/sysctl.h>
#include <net/if_mib.h>
#include <net/if.h>      // if_nametoindex
#include <string>

namespace network::iface {

static bool GetIfData_macos(const std::string& ifname, struct ifmibdata& out)
{
    unsigned ifindex = if_nametoindex(ifname.c_str());
    if (ifindex == 0) return false;

    int mib[6];
    mib[0] = CTL_NET;
    mib[1] = PF_LINK;
    mib[2] = NETLINK_GENERIC;
    mib[3] = IFMIB_IFDATA;
    mib[4] = static_cast<int>(ifindex);
    mib[5] = IFDATA_GENERAL;

    size_t len = sizeof(out);
    if (sysctl(mib, 6, &out, &len, nullptr, 0) != 0 || len < sizeof(out)) {
        return false;
    }
    return true;
}

void GetCounters(Counters& st)
{
    const std::string iface = network::iface::InterfaceName();

    struct ifmibdata ifmd{};
    if (!GetIfData_macos(iface, ifmd)) {
        return; // leave zeros if not found
    }

	st.rx_ok  = ifmd.ifmd_data.ifi_ipackets;
	st.rx_err = ifmd.ifmd_data.ifi_ierrors;
	st.rx_drp = ifmd.ifmd_data.ifi_iqdrops;  // input queue drops
	st.rx_ovr = 0;                           // not exposed
	
	st.tx_ok  = ifmd.ifmd_data.ifi_opackets;
	st.tx_err = ifmd.ifmd_data.ifi_oerrors;
	st.tx_drp = 0;                           // no ifi_oqdrops on macOS
	st.tx_ovr = 0;                           // not exposed

}

} // network::iface
#else
#include <string>
#include <sstream>
#include <fstream>
#include <regex>

namespace network::iface
{
static bool ReadFile(const std::string& path, std::string& out)
{
    std::ifstream ifs(path);
    if (!ifs) return false;
    std::ostringstream ss;
    ss << ifs.rdbuf();
    out = ss.str();
    return true;
}

void GetCounters(Counters& st)
{
    std::string data;
    if (!ReadFile("/proc/net/dev", data)) return;

    const std::string iface = network::iface::InterfaceName();

    std::istringstream in(data);
    std::string line;

    while (std::getline(in, line))
    {
        auto p = line.find(':');
        if (p == std::string::npos) continue;
        // trim left of line before comparing
        auto start = line.find_first_not_of(" \t");
        if (start == std::string::npos) continue;
        if (line.compare(start, iface.size(), iface) != 0) continue;

        std::istringstream fields(line.substr(p + 1));
        // Receive:
        uint64_t r_bytes, r_packets, r_errs, r_drop, r_fifo, r_frame, r_comp, r_mcast;
        // Transmit:
        uint64_t t_bytes, t_packets, t_errs, t_drop, t_fifo, t_colls, t_carrier, t_comp;

        if (!(fields >> r_bytes >> r_packets >> r_errs >> r_drop >> r_fifo >> r_frame >> r_comp >> r_mcast >> t_bytes >> t_packets >> t_errs >> t_drop >>
              t_fifo >> t_colls >> t_carrier >> t_comp))
        {
            return;
        }

        st.rx_ok = r_packets;
        st.rx_err = r_errs;
        st.rx_drp = r_drop;
        st.rx_ovr = r_fifo;

        st.tx_ok = t_packets;
        st.tx_err = t_errs;
        st.tx_drp = t_drop;
        st.tx_ovr = t_fifo;
        return;
    }
}
} // network::iface
#endif