#pragma once

#ifndef WINVER
#define WINVER 0x601
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x601
#endif
#ifndef _WIN32_WINDOWS
#define _WIN32_WINDOWS 0x601
#endif

#include <winsdkver.h>

#include "../helpers/foobar2000+atl.h"

#pragma warning(disable: 4996)

#include <CommCtrl.h>
#include <commoncontrols.h>
#include <atlframe.h>
#include <atlctrlx.h>
#include <atlctrls.h>

#include "../SDK/foobar2000.h"
#include "../SDK/cfg_var_legacy.h"
#include "../SDK/filesystem_helper.h"
#include "../SDK/album_art.h"

#include "../helpers/helpers.h"
#include "../helpers/atl-misc.h"

#include "../../libPPUI/win32_op.h"
#include "../../libPPUI/win32_utility.h"

#include "helpers/DarkMode.h"

#include <Windows.h>
#include <PowrProf.h>
#include <uxtheme.h>

#include <atlbase.h>
#include <atlstr.h>

// #define _WTL_NO_AUTOMATIC_NAMESPACE
#define _WTL_NO_CSTRING

#define BOOST_ALL_NO_LIB

#include <atlapp.h>
#include <atlcrack.h>
#include <atlddx.h>
#include <atlctrlx.h>
#include <atldlgs.h>

#include <string>
#include <vector>
#include <map>

#include <boost/algorithm/string.hpp>
#include <boost/any.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/detail/atomic_count.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/format.hpp>
#include <boost/function.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_map.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/signals2.hpp>
#include <boost/type_traits.hpp>
