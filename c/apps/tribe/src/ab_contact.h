#ifndef _AB_CONTACT_H_INCLUDED_
#define _AB_CONTACT_H_INCLUDED_ 1

typedef struct ab_contact_s
{
  int id;
  char const *fname;
  char const *lname;
  char const *email;
} ab_contact_t;

int ab_contact_create(ab_contact_t **pp_contact);
void ab_contact_destroy(ab_contact_t *contact);

void ab_contact_set_first_name(ab_contact_t *contact, char const *fname);
void ab_contact_set_last_name(ab_contact_t *contact, char const *lname);
void ab_contact_set_email(ab_contact_t *contact, char const *email);

char const *ab_contact_get_first_name(ab_contact_t *contact);
char const *ab_contact_get_last_name(ab_contact_t *contact);
char const *ab_contact_get_email(ab_contact_t *contact);

#endif  /* !defined(_AB_CONTACT_H_INCLUDED_) */
