#pragma once

#include <tuple>


class SOTMasterLockableType {
};


template <typename Outer>
struct InnerTypeIsBaseOfSOTMasterLockableT:
        std::is_base_of<SOTMasterLockableType,
                        typename Outer::SOT_MASTER_TYPE> {};