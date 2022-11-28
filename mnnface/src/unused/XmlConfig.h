#ifndef  __XML_CONFIG_H__
#define  __XML_CONFIG_H__
#include "tinyxml.h"
#include"Config.h"
#include"Log.h"




class CXmlConfig:public CLog
{
    public:
        CXmlConfig();
       virtual ~CXmlConfig();
    public:
        void Init(const char *strVerson,const char *strXmlname);
        HIKCAMPARAMLIST & GetHikParam();
        HIKPASSSYNCPARAM & GetPasssyncParam();
        HIKDOORCTLPARAM & GetDoorctlParam();
        HIKPERSONSYNCPARAM & GetPersonsyncParam();
        HIKREGIONSYNCPARAM & GetRegionsyncParam();
        HIKALARMSYNCPARAM & GetAlarmSyncParam();
        HIKPRIVISYNCPARAM & GetPriviSyncParam();
        SERVERPASSPARAM & GetPassParam();
        FACEDECTPARAM & GetFaceDectParam();
    protected:
        void LoadCameraListParam(TiXmlNode*pAlarmNode);
        void LoadPersonSyncParam(TiXmlNode*pAlarmNode);
        void LoadRegionSyncParam(TiXmlNode*pAlarmNode);
        void LoadDoorCtlParam(TiXmlNode*pAlarmNode);
        void LoadPassSyncParam(TiXmlNode*pAlarmNode);
        void LoadAlarmSyncParam(TiXmlNode*pAlarmNode);
        void LoadPriviSyncParam(TiXmlNode*pAlarmNode);
        HIKCAMPARAMLIST     m_campl;
        HIKPERSONSYNCPARAM  m_persp;
        HIKREGIONSYNCPARAM  m_regionsp;
        HIKDOORCTLPARAM     m_doorcp;
        HIKPASSSYNCPARAM    m_passsp;
        HIKALARMSYNCPARAM   m_almsp;
        HIKPRIVISYNCPARAM   m_prisp;
        SERVERPASSPARAM     m_passp;

};







#endif//__XML__CONFIG_H__
