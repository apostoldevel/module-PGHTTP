/*++

Program name:

  Apostol CRM

Module Name:

  PGHTTP.cpp

Notices:

  Module: Postgres Fetch

Author:

  Copyright (c) Prepodobny Alen

  mailto: alienufo@inbox.ru
  mailto: ufocomp@gmail.com

--*/

//----------------------------------------------------------------------------------------------------------------------

#include "Core.hpp"
#include "PGHTTP.hpp"
//----------------------------------------------------------------------------------------------------------------------

#define CONFIG_FILE_NAME     "conf/pg_http.conf"
#define SECTION_ENDPOINTS    "endpoints"

extern "C++" {

namespace Apostol {

    namespace Module {

        //--------------------------------------------------------------------------------------------------------------

        //-- CPGHTTP ---------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        CPGHTTP::CPGHTTP(CModuleProcess *AProcess): CFetchCommon(AProcess, "pg http", "module/PGHTTP") {
            CPGHTTP::InitMethods();
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPGHTTP::InitMethods() {
#if defined(_GLIBCXX_RELEASE) && (_GLIBCXX_RELEASE >= 9)
            m_Methods.AddObject(_T("GET")    , (CObject *) new CMethodHandler(true , [this](const auto& Connection) { DoGet(Connection); }));
            m_Methods.AddObject(_T("POST")   , (CObject *) new CMethodHandler(true , [this](const auto& Connection) { DoPost(Connection); }));
            m_Methods.AddObject(_T("OPTIONS"), (CObject *) new CMethodHandler(true , [this](const auto& Connection) { DoOptions(Connection); }));
            m_Methods.AddObject(_T("HEAD")   , (CObject *) new CMethodHandler(false, [this](const auto& Connection) { MethodNotAllowed(Connection); }));
            m_Methods.AddObject(_T("PUT")    , (CObject *) new CMethodHandler(true , [this](const auto& Connection) { DoPut(Connection); }));
            m_Methods.AddObject(_T("DELETE") , (CObject *) new CMethodHandler(true , [this](const auto& Connection) { DoDelete(Connection); }));
            m_Methods.AddObject(_T("TRACE")  , (CObject *) new CMethodHandler(false, [this](const auto& Connection) { MethodNotAllowed(Connection); }));
            m_Methods.AddObject(_T("PATCH")  , (CObject *) new CMethodHandler(true , [this](const auto& Connection) { DoPatch(Connection); }));
            m_Methods.AddObject(_T("CONNECT"), (CObject *) new CMethodHandler(false, [this](const auto& Connection) { MethodNotAllowed(Connection); }));
#else
            m_Methods.AddObject(_T("GET")    , (CObject *) new CMethodHandler(true , std::bind(&CPGHTTP::DoGet, this, _1)));
            m_Methods.AddObject(_T("POST")   , (CObject *) new CMethodHandler(true , std::bind(&CPGHTTP::DoPost, this, _1)));
            m_Methods.AddObject(_T("OPTIONS"), (CObject *) new CMethodHandler(true , std::bind(&CPGHTTP::DoOptions, this, _1)));
            m_Methods.AddObject(_T("HEAD")   , (CObject *) new CMethodHandler(false, std::bind(&CPGHTTP::MethodNotAllowed, this, _1)));
            m_Methods.AddObject(_T("PUT")    , (CObject *) new CMethodHandler(true , std::bind(&CPGHTTP::DoPut, this, _1)));
            m_Methods.AddObject(_T("DELETE") , (CObject *) new CMethodHandler(true , std::bind(&CPGHTTP::DoDelete, this, _1)));
            m_Methods.AddObject(_T("TRACE")  , (CObject *) new CMethodHandler(false, std::bind(&CPGHTTP::MethodNotAllowed, this, _1)));
            m_Methods.AddObject(_T("PATCH")  , (CObject *) new CMethodHandler(true , std::bind(&CPGHTTP::DoPatch, this, _1)));
            m_Methods.AddObject(_T("CONNECT"), (CObject *) new CMethodHandler(false, std::bind(&CPGHTTP::MethodNotAllowed, this, _1)));
#endif
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPGHTTP::PQGet(CHTTPServerConnection *AConnection, const CString &Path) {

            const auto &caRequest = AConnection->Request();

            CStringList SQL;

            const auto &caHeaders = HeadersToJson(caRequest.Headers).ToString();
            const auto &caParams = ParamsToJson(caRequest.Params).ToString();

            SQL.Add(CString()
                            .MaxFormatSize(256 + Path.Size() + caHeaders.Size() + caParams.Size())
                            .Format("SELECT * FROM http.get(%s, %s::jsonb, %s::jsonb);",
                                    PQQuoteLiteral(Path).c_str(),
                                    PQQuoteLiteral(caHeaders).c_str(),
                                    PQQuoteLiteral(caParams).c_str()
                            ));

            try {
                ExecSQL(SQL, AConnection);
            } catch (Delphi::Exception::Exception &E) {
                AConnection->CloseConnection(true);
                ReplyError(AConnection, CHTTPReply::bad_request, E.what());
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPGHTTP::PQPost(CHTTPServerConnection *AConnection, const CString &Path, const CString &Body) {

            const auto &caRequest = AConnection->Request();

            CStringList SQL;

            const auto &caHeaders = HeadersToJson(caRequest.Headers).ToString();
            const auto &caParams = ParamsToJson(caRequest.Params).ToString();
            const auto &caBody = Body.IsEmpty() ? "null" : PQQuoteLiteral(Body);

            SQL.Add(CString()
                            .MaxFormatSize(256 + Path.Size() + caHeaders.Size() + caParams.Size() + caBody.Size())
                            .Format("SELECT * FROM http.post(%s, %s::jsonb, %s::jsonb, %s::jsonb);",
                                    PQQuoteLiteral(Path).c_str(),
                                    PQQuoteLiteral(caHeaders).c_str(),
                                    PQQuoteLiteral(caParams).c_str(),
                                    caBody.c_str()
                            ));

            try {
                ExecSQL(SQL, AConnection);
            } catch (Delphi::Exception::Exception &E) {
                AConnection->CloseConnection(true);
                ReplyError(AConnection, CHTTPReply::bad_request, E.what());
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPGHTTP::PQPatch(CHTTPServerConnection* AConnection, const CString& Path, const CString& Body) {

            const auto &caRequest = AConnection->Request();

            CStringList SQL;

            const auto &caHeaders = HeadersToJson(caRequest.Headers).ToString();
            const auto &caParams = ParamsToJson(caRequest.Params).ToString();
            const auto &caBody = Body.IsEmpty() ? "null" : PQQuoteLiteral(Body);

            SQL.Add(CString()
                            .MaxFormatSize(256 + Path.Size() + caHeaders.Size() + caParams.Size() + caBody.Size())
                            .Format("SELECT * FROM http.patch(%s, %s::jsonb, %s::jsonb, %s::jsonb);",
                                    PQQuoteLiteral(Path).c_str(),
                                    PQQuoteLiteral(caHeaders).c_str(),
                                    PQQuoteLiteral(caParams).c_str(),
                                    caBody.c_str()
                            ));

            try {
                ExecSQL(SQL, AConnection);
            } catch (Delphi::Exception::Exception &E) {
                AConnection->CloseConnection(true);
                ReplyError(AConnection, CHTTPReply::bad_request, E.what());
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPGHTTP::PQPut(CHTTPServerConnection* AConnection, const CString& Path, const CString& Body) {

            const auto &caRequest = AConnection->Request();

            CStringList SQL;

            const auto &caHeaders = HeadersToJson(caRequest.Headers).ToString();
            const auto &caParams = ParamsToJson(caRequest.Params).ToString();
            const auto &caBody = Body.IsEmpty() ? "null" : PQQuoteLiteral(Body);

            SQL.Add(CString()
                            .MaxFormatSize(256 + Path.Size() + caHeaders.Size() + caParams.Size() + caBody.Size())
                            .Format("SELECT * FROM http.put(%s, %s::jsonb, %s::jsonb, %s::jsonb);",
                                    PQQuoteLiteral(Path).c_str(),
                                    PQQuoteLiteral(caHeaders).c_str(),
                                    PQQuoteLiteral(caParams).c_str(),
                                    caBody.c_str()
                            ));

            try {
                ExecSQL(SQL, AConnection);
            } catch (Delphi::Exception::Exception &E) {
                AConnection->CloseConnection(true);
                ReplyError(AConnection, CHTTPReply::bad_request, E.what());
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPGHTTP::PQDelete(CHTTPServerConnection* AConnection, const CString& Path, const CString& Body) {

            const auto &caRequest = AConnection->Request();

            CStringList SQL;

            const auto &caHeaders = HeadersToJson(caRequest.Headers).ToString();
            const auto &caParams = ParamsToJson(caRequest.Params).ToString();
            const auto &caBody = Body.IsEmpty() ? "null" : PQQuoteLiteral(Body);

            SQL.Add(CString()
                            .MaxFormatSize(256 + Path.Size() + caHeaders.Size() + caParams.Size() + caBody.Size())
                            .Format("SELECT * FROM http.delete(%s, %s::jsonb, %s::jsonb, %s::jsonb);",
                                    PQQuoteLiteral(Path).c_str(),
                                    PQQuoteLiteral(caHeaders).c_str(),
                                    PQQuoteLiteral(caParams).c_str(),
                                    caBody.c_str()
                            ));

            try {
                ExecSQL(SQL, AConnection);
            } catch (Delphi::Exception::Exception &E) {
                AConnection->CloseConnection(true);
                ReplyError(AConnection, CHTTPReply::bad_request, E.what());
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPGHTTP::DoGet(CHTTPServerConnection *AConnection) {

            const auto &caRequest = AConnection->Request();
            auto &Reply = AConnection->Reply();

            Reply.ContentType = CHTTPReply::json;

            const auto &path = caRequest.Location.pathname;

            if (path.IsEmpty()) {
                AConnection->SendStockReply(CHTTPReply::not_found);
                return;
            }

            PQGet(AConnection, path);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPGHTTP::DoPost(CHTTPServerConnection *AConnection) {

            const auto &caRequest = AConnection->Request();
            auto &Reply = AConnection->Reply();

            Reply.ContentType = CHTTPReply::json;

            const auto &path = caRequest.Location.pathname;

            if (path.IsEmpty()) {
                AConnection->SendStockReply(CHTTPReply::not_found);
                return;
            }

            const auto& caContentType = caRequest.Headers[_T("Content-Type")].Lower();
            const auto bContentJson = (caContentType.Find(_T("application/json")) != CString::npos);

            CJSON Json;
            if (!bContentJson) {
                ContentToJson(caRequest, Json);
            }

            const auto& caBody = bContentJson ? caRequest.Content : Json.ToString();

            PQPost(AConnection, path, caBody);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPGHTTP::DoPatch(CHTTPServerConnection* AConnection) {

            const auto &caRequest = AConnection->Request();
            auto &Reply = AConnection->Reply();

            Reply.ContentType = CHTTPReply::json;

            const auto &path = caRequest.Location.pathname;

            if (path.IsEmpty()) {
                AConnection->SendStockReply(CHTTPReply::not_found);
                return;
            }

            const auto& caContentType = caRequest.Headers[_T("Content-Type")].Lower();
            const auto bContentJson = (caContentType.Find(_T("application/json")) != CString::npos);

            CJSON Json;
            if (!bContentJson) {
                ContentToJson(caRequest, Json);
            }

            const auto& caBody = bContentJson ? caRequest.Content : Json.ToString();

            PQPatch(AConnection, path, caBody);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPGHTTP::DoPut(CHTTPServerConnection* AConnection) {

            const auto &caRequest = AConnection->Request();
            auto &Reply = AConnection->Reply();

            Reply.ContentType = CHTTPReply::json;

            const auto &path = caRequest.Location.pathname;

            if (path.IsEmpty()) {
                AConnection->SendStockReply(CHTTPReply::not_found);
                return;
            }

            const auto& caContentType = caRequest.Headers[_T("Content-Type")].Lower();
            const auto bContentJson = (caContentType.Find(_T("application/json")) != CString::npos);

            CJSON Json;
            if (!bContentJson) {
                ContentToJson(caRequest, Json);
            }

            const auto& caBody = bContentJson ? caRequest.Content : Json.ToString();

            PQPut(AConnection, path, caBody);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPGHTTP::DoDelete(CHTTPServerConnection* AConnection) {

            const auto &caRequest = AConnection->Request();
            auto &Reply = AConnection->Reply();

            Reply.ContentType = CHTTPReply::json;

            const auto &path = caRequest.Location.pathname;

            if (path.IsEmpty()) {
                AConnection->SendStockReply(CHTTPReply::not_found);
                return;
            }

            const auto& caContentType = caRequest.Headers[_T("Content-Type")].Lower();
            const auto bContentJson = (caContentType.Find(_T("application/json")) != CString::npos);

            CJSON Json;
            if (!bContentJson) {
                ContentToJson(caRequest, Json);
            }

            const auto& caBody = bContentJson ? caRequest.Content : Json.ToString();

            PQDelete(AConnection, path, caBody);
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPGHTTP::InitConfig(const CIniFile &IniFile, const CString &Section, CStringList &Config) {
            if (Section == SECTION_ENDPOINTS) {
                IniFile.ReadSectionValues(Section.c_str(), &Config);
                if (Config.Count() == 0)
                    Config.Add("/api/*");
            }
        }
        //--------------------------------------------------------------------------------------------------------------

        void CPGHTTP::Initialization(CModuleProcess *AProcess) {
            CFetchCommon::Initialization(AProcess);
            LoadConfig(Config()->IniFile().ReadString(SectionName().c_str(), "config", CONFIG_FILE_NAME), m_Profiles, InitConfig);
            auto& Config = m_Profiles[SECTION_ENDPOINTS];
            if (Config.Count() == 0)
                Config.Add("/api/*");
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CPGHTTP::Enabled() {
            if (m_ModuleStatus == msUnknown)
                m_ModuleStatus = Config()->IniFile().ReadBool(SectionName().c_str(), "enable", true) ? msEnabled : msDisabled;
            return m_ModuleStatus == msEnabled;
        }
        //--------------------------------------------------------------------------------------------------------------

        bool CPGHTTP::CheckLocation(const CLocation &Location) {
            return AllowedLocation(Location.pathname, m_Profiles[SECTION_ENDPOINTS]);
        }
        //--------------------------------------------------------------------------------------------------------------
    }
}
}