#pragma once
#include "../../../../Defines/ConstantDefs/Marcos.h"
#include "../../../../Defines/ConstantDefs/Types.h"
#include "../../../../TradingPlatformAPI/CTP6.3.15/ThostFtdcTraderApi.h"
#include "../../../../Defines/BaseDefs/IGatewayApi.h"
#include "../../../../tools/CommonMgr.h"
#include <string>
#include <vector>
#include <memory>
#include <thread>
#include <atomic>

USING_NS;

//CTP交易网关, 一方面实现CTP需要的回调函数, 另一方面提供标准化的交易接口
class CTPGateway : public IGatewayApi, public CThostFtdcTraderSpi
{
public:
    typedef HashMap<std::string> PositionMap;
    typedef std::shared_ptr<std::thread> StdThreadPtr;
private:
    //CTP交易接口 & 适配器回调接口
    CThostFtdcTraderApi* m_pCTPApi;         //CTP的交易API; 用来向CTP下达各种请求
    // IGatewaySpi*    m_sink;                 //回调类, 即Adapter;
    //子线程执行对CTP的查询请求
    StdThreadPtr    m_thrdWorker;
    bool            m_bStopped;
	//配置信息
    std::string		m_strBroker;            //经纪商
	std::vector<std::string> m_strFront;    //前置ip
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
    //网关运行标识
    TGConnectState    m_gatewayState;         //与CTP连接所处状态
    uint32_t        m_uFrontID;              //前置编号
    uint32_t        m_uSessionID;            //会话编号
    uint32_t        m_uTradingDay;          //当前交易日
    uint32_t        m_uMaxOrderRef;         //最大报单引用
    std::atomic<uint32_t>   m_uRequestID;   //发送给CTP的请求的ID(自行维护)




public:
    CTPGateway();
    virtual ~CTPGateway() override;

public:
    ////实现IGatewayApi, 即gateway对外提供的交易接口, 由adapter调用////
    
    //设置接口//
    virtual void init(Variant* cfg) override;
    virtual void release() override;
    // virtual void register_spi(IGatewaySpi* listener) override;
    void join();
    
    //执行交易命令前的准备工作: 与柜台连接、登录、确认结算单信息//
    virtual void connect() override;
    virtual void disconnect() override;
    virtual bool is_connected() override;
    virtual int login() override;
    virtual int logout() override;
    
    //交易命令接口//
    virtual int order_insert(Entrust* entrust) override;
    virtual int order_action(EntrustAction* action) override;
    virtual bool make_entrustID(char* buffer, int length) override;
    
    //查询信息接口//
    virtual int query_account() override;
    virtual int query_positions() override;
    virtual int query_orders() override;
    virtual int query_trades() override;
    virtual int query_settlement(uint32_t uDate) override;

private:
    ////gateway与CTP交互的内部逻辑, gateway直接调用CTP提供的Api的部分写在这里////

    //执行交易命令前的准备工作: 与柜台连接、登录、确认结算单信息//
    void req_connect();          //请求连接
    void req_authenticate();     //请求认证
    void req_login();            //请求登录
    void req_query_settlement_confirm();    //请求查询结算单确认情况
    void req_settlement_confirm();          //请求确认结算单

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
	virtual void OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
	virtual void OnRtnOrder(CThostFtdcOrderField *pOrder) override;
	virtual void OnRtnTrade(CThostFtdcTradeField *pTrade) override;
	virtual void OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo) override;
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
    //检验CTP的回应是否为错误//
    bool is_err_rspInfo(CThostFtdcRspInfoField *pRspInfo);


    
};




// 创建CTP API实例：调用 CreateFtdcTraderApi()或者CreateFtdcMdApi()函数创建CTP API 实例，这个实例用于连接到CTP服务器并进行交易或行情订阅。

// 注册回调函数：使用RegisterSpi()函数注册回调函数，这些回调函数用于处理交易或行情订阅相关的事件，比如登录成功、查询结果返回等。

// 连接到CTP服务器：使用Init()函数初始化 CTP API 实例，并使用RegisterFront()函数注册CTP服务器的地址和端口号，然后使用Init()函数连接到CTP服务器。

// 用户登录：使用ReqUserLogin()函数登录到 CTP 服务器，需要提供经纪公司代码、用户名、密码等信息。

// 查询结算单信息：使用ReqQrySettlementInfo()函数查询结算单信息，需要提供经纪公司代码、投资者代码等信息。

// 确认结算单信息：使用ReqSettlementInfoConfirm()函数确认结算单信息，需要提供经纪公司代码、投资者代码等信息。