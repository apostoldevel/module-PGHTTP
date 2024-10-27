/*++

Program name:

  Apostol CRM

Module Name:

  PGHTTP.hpp

Notices:

  Module: Postgres HTTP

Author:

  Copyright (c) Prepodobny Alen

  mailto: alienufo@inbox.ru
  mailto: ufocomp@gmail.com

--*/

#ifndef APOSTOL_PQ_HTTP_HPP
#define APOSTOL_PQ_HTTP_HPP
//----------------------------------------------------------------------------------------------------------------------

#include "FetchCommon.hpp"
//----------------------------------------------------------------------------------------------------------------------

extern "C++" {

namespace Apostol {

    namespace Module {

        //--------------------------------------------------------------------------------------------------------------

        //-- CPGHTTP ---------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CPGHTTP: public CFetchCommon {
        private:

            CStringListPairs m_Profiles;

            void InitMethods() override;

            void PQGet(CHTTPServerConnection *AConnection, const CString &Path);
            void PQPost(CHTTPServerConnection *AConnection, const CString &Path, const CString &Body);

            static void InitConfig(const CIniFile &IniFile, const CString &Section, CStringList &Config);

        protected:

            void DoGet(CHTTPServerConnection *AConnection) override;
            void DoPost(CHTTPServerConnection *AConnection);

        public:

            explicit CPGHTTP(CModuleProcess *AProcess);

            ~CPGHTTP() override = default;

            static CPGHTTP *CreateModule(CModuleProcess *AProcess) {
                return new CPGHTTP(AProcess);
            }

            void Initialization(CModuleProcess *AProcess) override;

            bool Enabled() override;

            bool CheckLocation(const CLocation &Location) override;

        };

    }
}

using namespace Apostol::Module;
}
#endif //APOSTOL_PQ_HTTP_HPP
