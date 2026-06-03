#include "NblFilterRef.hpp"
#include "ApplicationContext.hpp"

#include <Data/Model.hpp>

i32 NblFilterRef::loadInstallations() {

    ApplicationContext* context = ApplicationContext::instance();

    nbl::DatabaseConnection* connection = context->connection();
    if(!connection->isValid()) {
        return -1;
    }
    
    _mInstallations.clear();
    nbl::lookupInstallationsForFilter(connection, filterId(), [](void* userData, const nbl::InstallationMeta& meta) {

        NblInstallationRef ref(meta.id, 
            QString::fromUtf8(meta.title.data, meta.title.length), 
            QString::fromUtf8(meta.poster.data, meta.poster.length)
        );

        static_cast<QList<NblInstallationRef>*>(userData)->append(ref);

        return nbl::Result::SUCCESS;
}, 0, 0, &_mInstallations);

    installationsUpdated();

    return 0;
}
