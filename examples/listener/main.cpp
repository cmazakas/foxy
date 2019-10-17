#include <pqxx/connection.hxx>

int
main()
{
  pqxx::connection db_conn{"postgresql://postgres@localhost:5432"};
  return 0;
}
