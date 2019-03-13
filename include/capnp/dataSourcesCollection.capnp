@0xc4d1311aba086738;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("dataSourcesCollection");

using DataSource = import "dataSource.capnp".DataSource;

struct DataSourcesCollection {
  dataSources @0 :List(DataSource);
}
