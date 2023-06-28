#pragma once
#include "Defines/ConstantDefs/Marcos.h"
#include "Defines/ConstantDefs/Types.h"
#include "TradingPlatformAPI/CTP6.3.15/ThostFtdcTraderApi.h"
#include "Defines/BaseDefs/ITradeGatewayApi.h"
#include "Defines/BaseDefs/ITradeGatewaySink.h"

#include <string>
#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

USING_NS;

//CTP交易网关, 一方面实现CTP需要的回调函数, 另一方面提供标准化的交易接口
class CTPGateway : public ITradeGatewayApi, public CThostFtdcTraderSpi
{
public:
    typedef HashMap<std::string> PositionMap;
    typedef std::shared_ptr<std::thread> StdThreadPtr;
    typedef std::atomic<TGConnectState> ConnectState;
    typedef std::atomic<uint32_t> RequestID;
    typedef std::atomic<uint32_t> OrderRef;
public:
    //CTP交易接口 & 适配器回调接口
    CThostFtdcTraderApi* m_pCTPApi;         //CTP的交易API; 用来向CTP下达各种请求
    // ITradeGatewaySink*    m_sink;                 //回调类, 即Adapter;
    //子线程执行对CTP的查询请求
    StdThreadPtr    m_thrdWorker;
    bool            m_bStopped;
	//配置信息
    std::string		m_strBroker;            //经纪商
	std::string     m_strFront;             //前置ip
	std::string		m_strUser;              //用户名
	std::string		m_strPass;              //密码
	std::string		m_strAppID;             //客户端appid, 期货公司申请
	std::string		m_strAuthCode;          //客户端认证码, 期货公司申请
    std::string     m_strFlowDir;           //流文件根目录
	std::string		m_strProdInfo;          //客户端产品信息, 如软件开发商、版本号等, 如wondertrader; 可不填写    
    //查询结果
    PositionMap*    m_mapPosition;
    Array*          m_ayTrades;
    Array*          m_ayOrders;
    Array*          m_ayPosDetail;
    //合约基础信息读取
    CommonMgr*      m_cmgr;
    //网关连接状态标识
    std::mutex      m_mtxConnect;           //连接状态互斥量
    std::condition_variable m_cvConnect;    //连接状态条件变量
    ConnectState    m_gatewayState;         //与CTP连接所处状态

    RequestID       m_uRequestID;           //发送给CTP的请求的ID(自行维护)
    OrderRef        m_uOrderRef;            //报单引用(自行维护)

    uint32_t        m_uFrontID;             //前置编号
    uint32_t        m_uSessionID;           //会话编号
    uint32_t        m_uTradingDay;          //当前交易日
    uint32_t        m_uMaxOrderRef;         //最大报单引用

public:
    CTPGateway();
    virtual ~CTPGateway() override;

public:
    ////实现IGatewayApi, 即gateway对外提供的交易接口, 由adapter调用////
    
    //设置接口//
    virtual int init(Variant* cfg) override;
    virtual int release() override;
    // virtual void register_spi(IGatewaySpi* listener) override;
    //TODO以下两个函数改变位置
    void add_cmgr(CommonMgr* cmgr) {m_cmgr = cmgr;}
    void join() {m_pCTPApi->Join();};
    
    //执行交易命令前的准备工作: 与柜台连接、登录、确认结算单信息//
    virtual int  connect() override;
    virtual int  disconnect() override;
    virtual bool is_connected() override;
    virtual int  login() override;
    virtual int  logout() override;
    virtual int  confirm() override;
    
    //交易命令接口//
    bool         generate_entrustID(char* buffer, int length) override;//新产生一个委托
    virtual int  order_insert(Entrust* entrust) override;
    virtual int  order_action(EntrustAction* action) override;
    
    
    //查询信息接口//
    virtual int  query_account() override;
    virtual int  query_positions() override;
    virtual int  query_orders() override;
    virtual int  query_trades() override;
    virtual int  query_settlement(uint32_t uDate) override;

private:
    ////gateway与CTP交互的内部逻辑, gateway直接调用CTP提供的Api的部分写在这里////

    //执行交易命令前的准备工作: 与柜台连接、登录、确认结算单信息//
    void req_connect();                     //请求连接
    void req_authenticate();                //请求认证
    void req_login();                       //请求登录
    void req_query_settlement_confirm();    //请求查询结算单确认情况
    void req_settlement_confirm();          //请求确认结算单
    void req_order_insert(Entrust* entrust);//请求报单
    void req_order_action(EntrustAction* action);//请求撤单
private:
    ////CTP以回调函数的参数的形式传递报文, 通过实现CTP回调函数来接收报文和执行下一步操作////


	//执行交易命令前的准备工作: 与柜台连接、登录、确认结算单信息//
    virtual void OnFrontConnected() override;
	virtual void OnFrontDisconnected(int nReason) override;
	virtual void OnRspAuthenticate(CThostFtdcRspAuthenticateField *pRspAuthenticateField, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
	virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
	virtual void OnRspQrySettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
	virtual void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
	virtual void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
	//CTP接受交易命令后的回报
    virtual void OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
	virtual void OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo) override;
    virtual void OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
	virtual void OnRtnOrder(CThostFtdcOrderField *pOrder) override;
	virtual void OnRtnTrade(CThostFtdcTradeField *pTrade) override;
    //CTP接受查询命令后的回报
    virtual void OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
	virtual void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;	
	virtual void OnRspQrySettlementInfo(CThostFtdcSettlementInfoField *pSettlementInfo, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
	virtual void OnRspQryTrade(CThostFtdcTradeField *pTrade, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
	virtual void OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
    //CTP的其他回调函数
    virtual void OnHeartBeatWarning(int nTimeLapse) override;
	virtual void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
	virtual void OnRtnInstrumentStatus(CThostFtdcInstrumentStatusField *pInstrumentStatus) override;



private:
    ////gateway实现的过程中常用的逻辑封装写在这里////
    
    //产生请求编号//
    uint32_t generate_requestID();
    
    bool     parse_entrustID(char* buffer, uint32_t uFrontID, uint32_t uSessionID, uint32_t uOrderRef);//由CTP回报解析出委托
    //提取编号中的信息//
    bool     extract_entrustID(const char* pEntrustID, uint32_t& uFrontID, uint32_t& uSessionID, uint32_t& uOrderRef);
    //检验CTP的回应是否为错误//
    bool     is_err_rspInfo(CThostFtdcRspInfoField *pRspInfo);    
    //系统内部格式转换为CTP格式//
    int      price_type_to_CTP(PriceType ptype, bool isCFFEX = false);
    int      direction_type_to_CTP(DirectionType dtype, OffsetType otype);
    int      offset_type_to_CTP(OffsetType otype);
    int      action_flag_to_CTP(ActionFlag flag);
    //CTP格式转换为系统内部格式//
    PriceType     price_type_to_my(TThostFtdcOrderPriceTypeType priceType);
    DirectionType direction_type_to_my(TThostFtdcDirectionType dirType, TThostFtdcOffsetFlagType offsetType);
    OffsetType    offset_type_to_my(TThostFtdcOffsetFlagType offsetType);
    OrderState    order_state_to_my(TThostFtdcOrderStatusType orderStatus);
    //CTP格式数据结构转换为系统内部格式//
    OrderInfo* orderInfo_to_my(CThostFtdcOrderField* orderField);
    Entrust*   entrust_to_my(CThostFtdcInputOrderField* inputOrder);
    TradeInfo* tradeInfo_to_my(CThostFtdcTradeField* tradeField);
};