#include "common.h"

#include <stdio.h>

#include "ab_contact.h"
#include "fort.h"

int print_contact_list(alpm_list_t *contact_list) {
  ft_table_t *table;
  alpm_list_t *cur;

  ft_set_default_border_style(FT_BASIC2_STYLE);
  table = ft_create_table();
  ft_set_cell_prop(table, FT_ANY_ROW, 0, FT_CPROP_TEXT_ALIGN, FT_ALIGNED_CENTER);
  ft_set_cell_prop(table, FT_ANY_ROW, 1, FT_CPROP_TEXT_ALIGN, FT_ALIGNED_LEFT);

  ft_set_cell_prop(table, 0, FT_ANY_COLUMN, FT_CPROP_ROW_TYPE, FT_ROW_HEADER);
  ft_write_ln(table, "ID", "First Name", "Last Name", "Email");

  for (cur = contact_list; cur; cur = alpm_list_next(cur)) {
    ab_contact_t *contact = (ab_contact_t *)cur->data;
    int length = snprintf(NULL, 0, "%d", contact->id);
    char* idstr = malloc(length + 1);
    snprintf(idstr, length + 1, "%d", contact->id);
    ft_write_ln(table, idstr, contact->fname, contact->lname, contact->email);
    free(idstr);
  }

  printf("%s\n", ft_to_string(table));
  ft_destroy_table(table);

  return 0;
}
