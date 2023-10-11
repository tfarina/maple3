#include "commands.h"

#include "ab.h"
#include "common.h"
#include "dirs.h"
#include "util.h"

int cmd_list(int argc, char **argv) {
  int rc;
  char *dbdir;

  dirs_init();

  if (!ensure_data_dir()) {
    return 1;
  }

  dbdir = dirs_get_user_data_dir();

  rc = ab_init(dbdir);
  if (rc < 0) {
    return 1;
  }

  rc = ab_load_contacts();
  if (rc < 0) {
    return 1;
  }

  rc = print_contact_list(ab_get_contact_list());

  ab_fini();

  return rc;
}
