set(3rdparty_include ${3rdparty_include} ${3rdparty_path}/CTP/v6.7.11_20250617_api_traderapi_se_linux64)
set(3rdparty_lib ${3rdparty_lib} ${3rdparty_path}/CTP/v6.7.11_20250617_api_traderapi_se_linux64)
#set(3rdparty_include ${3rdparty_include} ${3rdparty_path}/CTP/v6.7.9_P1_CP_20250225/include)
#set(3rdparty_lib ${3rdparty_lib} ${3rdparty_path}/CTP/v6.7.9_P1_CP_20250225/lib)

#set(3rdparty_include ${3rdparty_include} ${3rdparty_path}/CTP/XONE_API_v1.2.3.6_hotfix2/include)
#set(3rdparty_lib ${3rdparty_lib} ${3rdparty_path}/CTP/XONE_API_v1.2.3.6_hotfix2/lib)
set(3rdparty_so ${3rdparty_so} -lthostmduserapi_se -lthosttraderapi_se -ldl)