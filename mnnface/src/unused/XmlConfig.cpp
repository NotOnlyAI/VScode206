#include "XmlConfig.h"

/*<?xml version="1.0" encoding="utf-8"?>

<Config>
  <AlarmServer>
    <CameraList>
      <Camera>
        <name>机房1点位</name>
        <ip>172.16.7.152</ip>
        <uid>admin</uid>
        <pwd>12345</pwd>
        <port>8000</port>
        <desc>用于人脸识别的相机 放置在机房1内部</desc>
      </Camera>
      <Camera>
        <name>机房２点位</name>
        <ip>172.16.7.142</ip>
        <uid>admin</uid>
        <pwd>12345</pwd>
        <port>8000</port>
        <desc>用于人脸识别的相机 放置在机房２内部</desc>
      </Camera>
    </CameraList>
    <SyncParam>
      <url>http://xxx.xxx.xxx/12312asdf</url>
    </SyncParam>
    <DoorControl>
      <url>http://xxx.xxx.xxx/12312asdf</url>
    </DoorControl>
  </AlarmServer>
</Config>*/



CXmlConfig::CXmlConfig()
{
    CLog::InitLog(__FUNCTION__);
}

CXmlConfig::~CXmlConfig()
{

}

HIKCAMPARAMLIST & CXmlConfig::GetHikParam( )
{

    return m_campl;
}


HIKPASSSYNCPARAM & CXmlConfig::GetPasssyncParam()
{

    return m_passsp;
}

SERVERPASSPARAM & CXmlConfig::GetPassParam()
{

    return m_passp;
}



HIKDOORCTLPARAM & CXmlConfig::GetDoorctlParam()
{
    return m_doorcp;

}

HIKPERSONSYNCPARAM & CXmlConfig::GetPersonsyncParam()
{
    return m_persp;

}
HIKREGIONSYNCPARAM & CXmlConfig::GetRegionsyncParam()
{
    return m_regionsp;

}
HIKALARMSYNCPARAM & CXmlConfig::GetAlarmSyncParam()
{
    return m_almsp;

}
HIKPRIVISYNCPARAM & CXmlConfig::GetPriviSyncParam()
{
    return m_prisp;

}
void CXmlConfig::Init(const char *strVerson, const char *strXmlname)
{


    TiXmlDocument doc(strXmlname);
    bool loadOkay = doc.LoadFile();

    if ( !loadOkay )
    {
        ERR( "Could not load config file %s. Error='%s'. Exiting.",strXmlname, doc.ErrorDesc() );
        exit( 1 );
    }
    TiXmlNode* pNode =  doc.FirstChild( "Config" );
    if( NULL == pNode )
    {
        ERR( "Find Config in Cofig.xml failed " );
        return;
    }

    pNode= pNode->FirstChild(strVerson);
    if( NULL == pNode )
    {
        ERR( "Find Config\\%s in Config.xml failed ",strVerson );
        return;
    }

    pNode= pNode->FirstChild("AlarmServer");
    if( NULL == pNode )
    {
        ERR( "Find Config\\AlarmServer in Config.xml failed " );
        return;
    }
    LoadCameraListParam(pNode);
    LoadPersonSyncParam(pNode);
    LoadPriviSyncParam(pNode);
    LoadDoorCtlParam(pNode);
    LoadPassSyncParam(pNode);
    LoadAlarmSyncParam(pNode);
    LoadRegionSyncParam(pNode);
}
void  CXmlConfig::LoadCameraListParam(TiXmlNode*pNode)
{
    pNode = pNode->FirstChild("CameraList");
    if( NULL == pNode )
    {
        ERR( "Find Config\\AlarmServer\\CameraList in Config.xml failed " );
        return;
    }

    TiXmlNode *pCam = pNode->FirstChild("Camera");
    if( NULL == pCam )
    {
        ERR( "Find Config\\AlarmServer\\CameraList\\Camera in Config.xml failed " );

        return;
    }



    while( pCam)
    {
        HIKCAMPARAM param;

        TiXmlElement *p = pCam->FirstChildElement("name");
        if( NULL == pNode )
        {
            ERR( "Find Config\\AlarmServer\\CameraList\\Cameraame in Config.xml failed " );
            return;
        }

        const char * str= p->GetText();
        strcpy(param.szName,str);


        p = pCam->FirstChildElement("ip");
        if( NULL == p )
        {
            ERR( "Find Config\\AlarmServer\\CameraList\\Camera\\ip in Config.xml failed " );
            return;
        }
        str= p->GetText();
        strcpy(param.szIp,str);

        p = pCam->FirstChildElement("uid");
        if( NULL == p )
        {
            ERR("Find Config\\AlarmServer\\CameraList\\Camera\\uid in Config.xml failed ");
            return;
        }
        str= p->GetText();
        strcpy(param.szUid,str);

        p = pCam->FirstChildElement("pwd");
        if( NULL == p )
        {
            ERR( "Find Config\\AlarmServer\\CameraList\\Camera\\pwd in Config.xml failed " );
            return;
        }
        str= p->GetText();
        strcpy(param.szPwd,str);

        p = pCam->FirstChildElement("port");
        if( NULL == p )
        {
            ERR( "Find Config\\AlarmServer\\CameraList\\Camera\\port in Config.xml failed " );
            return;
        }
        str= p->GetText();
        param.nPort=atoi(str);

        p = pCam->FirstChildElement("save");
        if( NULL == p )
        {
            ERR( "Find Config\\AlarmServer\\CameraList\\Camera\\save in Config.xml failed " );
            return;
        }
        str= p->GetText();
        if( strcmp("yes",str)== 0 )
        {
            param.bSave = true;
        }
        else
        {
            param.bSave = false;
        }


        p = pCam->FirstChildElement("desc");
        if( NULL == p )
        {
            ERR( "Find Config\\AlarmServer\\CameraList\\Camera\\desc in Config.xml failed " );
            return;
        }
        str= p->GetText();
        strcpy(param.szDesc,str);

        p=pCam->FirstChildElement("aiboxip");
        if( NULL == p )
        {
            ERR( "Find Config\\AlarmServer\\CameraList\\Camera\\aiboxip in Config.xml failed " );
            return;
        }
        sprintf(param.fp.szIp,p->GetText());


        p=pCam->FirstChildElement("aiboxport");
        if( NULL == p )
        {
            ERR( "Find Config\\AlarmServer\\CameraList\\Camera\\aiboxport in Config.xml failed " );
            return;
        }
        param.fp.nPort = atoi(p->GetText());

        p = pCam->FirstChildElement("enable");
        if( NULL == p )
        {
            ERR( "Find Config\\AlarmServer\\CameraList\\Camera\\save in Config.xml failed " );
            return;
        }
        str= p->GetText();
        if( strcmp("yes",str)== 0)
        {
             m_campl.push_back(param);
        }

        pCam = pCam->NextSibling();
    }
}
void  CXmlConfig::LoadRegionSyncParam(TiXmlNode*pAlarmNode)
{
    TiXmlElement *pItem=NULL;
    TiXmlNode *pRoot = pAlarmNode->FirstChild("SyncRegion");
    if( NULL == pRoot){ ERR("LoadPersonSyncParam failed of find \\AlarmServer\\SyncRegion!");return;}

    TiXmlNode *pRootRegin = pRoot->FirstChild("rootregion");
    if( NULL == pRootRegin){ ERR("LoadPersonSyncParam failed of find \\AlarmServer\\SyncRegion\\rootregion!");return;}
    pItem = pRootRegin->FirstChildElement("url");
    if( NULL == pItem){ ERR("LoadPersonSyncParam failed of find \\AlarmServer\\SyncRegion\\rootregion\\url!");return;}
    strcpy(m_regionsp.szRootRegionUrl,pItem->GetText());

    TiXmlNode *pSubRegions = pRoot->FirstChild("subregions");
    if( NULL == pSubRegions){ ERR("LoadPersonSyncParam failed of find \\AlarmServer\\SyncRegion\\subregions!");return;}
    pItem = pSubRegions->FirstChildElement("url");
    if( NULL == pItem){ ERR("LoadPersonSyncParam failed of find \\AlarmServer\\SyncRegion\\subregions\\url!");return;}
    strcpy(m_regionsp.szSubRegionsurl,pItem->GetText());
}

void  CXmlConfig::LoadPersonSyncParam(TiXmlNode*pAlarmNode)
{
    TiXmlElement *pItem=NULL;
    TiXmlNode *pSycPersonNode = pAlarmNode->FirstChild("SyncPerson");
    if( NULL == pSycPersonNode){ ERR("LoadPersonSyncParam failed of find \\AlarmServer\\SyncPerson!");return;}

    TiXmlNode * pRootOrg= pSycPersonNode->FirstChildElement("rootorg");
    if( NULL == pRootOrg){ ERR("LoadPersonSyncParam failed of find \\AlarmServer\\SyncPerson\\rootorg!");return;}
    pItem = pRootOrg->FirstChildElement("url");
    if( NULL == pItem){ ERR("LoadPersonSyncParam failed of find \\AlarmServer\\SyncPerson\\rootorg\\url!");return;}
    strcpy(m_persp.szRootOrgUrl,pItem->GetText());

    TiXmlNode * pSubOrg= pSycPersonNode->FirstChildElement("suborg");
    if( NULL == pSubOrg){ ERR("LoadPersonSyncParam failed of find \\AlarmServer\\SyncPerson\\suborg!");return;}
    pItem = pSubOrg->FirstChildElement("url");
    if( NULL == pItem){ ERR("LoadPersonSyncParam failed of find \\AlarmServer\\SyncPerson\\suborg\\url!");return;}
    strcpy(m_persp.szSubOrgUrl,pItem->GetText());

    TiXmlNode * pAllOrg= pSycPersonNode->FirstChildElement("allorg");
    if( NULL == pAllOrg){ ERR("LoadPersonSyncParam failed of find \\AlarmServer\\SyncPerson\\allorg!");return;}
    pItem = pAllOrg->FirstChildElement("url");
    if( NULL == pItem){ ERR("LoadPersonSyncParam failed of find \\AlarmServer\\SyncPerson\\allorg\\url!");return;}
    strcpy(m_persp.szAllOrgUrl,pItem->GetText());

    TiXmlNode * pPersonInOrgNode= pSycPersonNode->FirstChildElement("personlist");
    if( NULL == pPersonInOrgNode){ ERR("LoadPersonSyncParam failed of find \\AlarmServer\\SyncPerson\\personinorg!");return;}
    pItem = pPersonInOrgNode->FirstChildElement("url");
    if( NULL == pItem){ ERR("LoadPersonSyncParam failed of find \\AlarmServer\\SyncPerson\\personinorg\\url!");return;}
    strcpy(m_persp.szPersonInOrgUrl,pItem->GetText());

}

void  CXmlConfig::LoadDoorCtlParam(TiXmlNode*pAlarmNode)
{
    TiXmlElement *pItem=NULL;
    TiXmlNode *p = NULL;

    TiXmlNode *pDoor = pAlarmNode->FirstChild("DoorControl");
    if( NULL == pDoor){ ERR("LoadPersonSyncParam failed of find \\AlarmServer\\DoorControl!");return;}

    p = pDoor->FirstChild("control");
    if( NULL == p){ ERR("LoadPersonSyncParam failed of find \\AlarmServer\\DoorControl\\control!");return;}
    pItem = p->FirstChildElement("url");
    if( NULL == pItem){ ERR("LoadPersonSyncParam failed of find \\AlarmServer\\DoorControl\\control\\url!");return;}
    strcpy(m_doorcp.szDoorCtrlUrl,pItem->GetText());

    p = pDoor->FirstChild("outcmd");
    if( NULL == p){ ERR("LoadPersonSyncParam failed of find \\AlarmServer\\DoorControl\\outcmd!");return;}
    pItem = p->FirstChildElement("url");
    if( NULL == pItem){ ERR("LoadPersonSyncParam failed of find \\AlarmServer\\DoorControl\\outcmd\\url!");return;}
    strcpy(m_doorcp.szOutCmdUrl,pItem->GetText());

}

void  CXmlConfig::LoadPassSyncParam(TiXmlNode*pAlarmNode)
{
    TiXmlElement *pItem=NULL;
    TiXmlNode *p = NULL;

    p= pAlarmNode->FirstChild("SyncPass");
    if( NULL == p){ ERR("LoadPersonSyncParam failed of find \\AlarmServer\\SyncPass!");return;}

    p = p->FirstChild("eventlist");
    if( NULL == p){ ERR("LoadPersonSyncParam failed of find \\AlarmServer\\SyncPass\\eventlist!");return;}
    pItem = p->FirstChildElement("url");
    if( NULL == pItem){ ERR("LoadPersonSyncParam failed of find \\AlarmServer\\SyncPass\\eventlist\\url!");return;}
    strcpy(m_passsp.szEventlistUrl,pItem->GetText());
}

void  CXmlConfig::LoadAlarmSyncParam(TiXmlNode*pAlarmNode)
{
    TiXmlElement *pItem=NULL;
    TiXmlNode *p = NULL;

    TiXmlNode * pSync = pAlarmNode->FirstChild("SyncAlarm");
    if( NULL == pSync){ ERR("LoadPersonSyncParam failed of find \\AlarmServer\\SyncAlarm!");return;}

    p = pSync->FirstChild("response");
    if( NULL == p){ ERR("LoadPersonSyncParam failed of find \\AlarmServer\\SyncAlarm\\response!");return;}
    pItem = p->FirstChildElement("url");
    if( NULL == pItem){ ERR("LoadPersonSyncParam failed of find \\AlarmServer\\SyncAlarm\\response\\url!");return;}
    strcpy(m_almsp.szResponsuRL,pItem->GetText());


    p = pSync->FirstChild("request");
    if( NULL == p){ ERR("LoadPersonSyncParam failed of find \\AlarmServer\\SyncAlarm\\request!");return;}
    pItem = p->FirstChildElement("url");
    if( NULL == pItem){ ERR("LoadPersonSyncParam failed of find \\AlarmServer\\SyncAlarm\\request\\url!");return;}
    strcpy(m_almsp.szRequestuRL,pItem->GetText());

    p=pSync->FirstChild("eventlist");
    if( NULL == p ){ ERR("LoadPersonSyncParam failed of find \\AlarmServer\\SyncAlarm\\eventlist!");return;}

    p=p->FirstChild();
    while(p)
    {
        HIKEVENT e;

        TiXmlElement *pele=p->FirstChildElement("name");
        if( NULL == pele ){ ERR("LoadPersonSyncParam failed of find \\AlarmServer\\SyncAlarm\\eventlist\\name!");return;}
        strcpy(e.szName,pele->GetText());

        pele=p->FirstChildElement("code");
        if( NULL == pele ){ ERR("LoadPersonSyncParam failed of find \\AlarmServer\\SyncAlarm\\eventlist\\code!");return;}
        e.nCode=atoi(pele->GetText());

        m_almsp.eventlist.push_back(e);

        p=p->NextSiblingElement();
    }
}
void CXmlConfig::LoadPriviSyncParam(TiXmlNode *pAlarmNode)
{
    TiXmlElement *pItem=NULL;
    TiXmlNode *p = NULL;

    p= pAlarmNode->FirstChild("SyncPrivili");
    if( NULL == p){ ERR("LoadPersonSyncParam failed of find \\AlarmServer\\SyncPrivili!");return;}

    p = p->FirstChild("privili");
    if( NULL == p){ ERR("LoadPersonSyncParam failed of find \\AlarmServer\\SyncPrivili\\privili!");return;}
    pItem = p->FirstChildElement("url");
    if( NULL == pItem){ ERR("LoadPersonSyncParam failed of find \\AlarmServer\\SyncPrivili\\privili\\url!");return;}
    strcpy(m_prisp.szuRL,pItem->GetText());

}
