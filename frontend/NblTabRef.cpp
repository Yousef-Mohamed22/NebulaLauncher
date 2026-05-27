#include "NblTabRef.hpp"
#include "ApplicationContext.hpp"
#include "Data/Model.hpp"

i32 NblTabRef::loadFilters() {

    ApplicationContext* context = ApplicationContext::instance();

    nbl::DatabaseConnection* connection = context->connection();
    if(!connection->isValid()) {
        return -1;
    }
    
    _mFilters.clear();
    nbl::getFiltersForTab(connection, tabId(), [](void* userData, i64 filterId, nbl::StringView name, nbl::StringView icon, const nbl::InstallationSearchCriteria& criteria, bool multiLine) {

        NblFilterRef ref(filterId, 
            QString::fromUtf8(name.data, name.length), 
            QString::fromUtf8(icon.data, icon.length),
            multiLine
        );

        static_cast<QList<NblFilterRef>*>(userData)->append(ref);

        return nbl::Result::SUCCESS;
}, &_mFilters);

    filtersUpdated();

    return 0;
}
