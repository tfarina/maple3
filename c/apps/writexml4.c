#include <stdio.h>

#include "third_party/mxml/mxml.h"
#include "third_party/libuuid/uuid.h"

#define ELEM_FOLDER "folder"
#define ELEM_ITEM_LIST "item-list"
#define ELEM_ITEM "item"
#define ATTR_UID "uid"
#define ATTR_NAME "name"
#define ATTR_TYPE "type"
#define ATTR_TYPE_VAL_FOLDER "folder"

typedef struct _ABItem ABItem;
struct _ABItem {
  char *uid;
  char *name;
  ABItem *parent;
};

typedef struct _ABFolder ABFolder;
struct _ABFolder {
  ABItem base;
  int is_root;
};

static ABFolder *
addrbook_folder_create(void)
{
  ABFolder *folder;

  folder = malloc(sizeof(ABFolder));
  if (folder == NULL) {
    return NULL;
  }

  ((ABItem *)folder)->uid = NULL;
  ((ABItem *)folder)->name = NULL;
  folder->is_root = 0;

  return folder;
}

static void
addrbook_folder_destroy(ABFolder *folder)
{
  if (folder == NULL) {
    return;
  }

  ((ABItem *)folder)->uid = NULL;
  ((ABItem *)folder)->name = NULL;
  folder->is_root = 0;

  free(folder);
}

int main(int argc, char **argv)
{
  mxml_node_t *xml = NULL;
  mxml_node_t *abook = NULL;
  mxml_node_t *folder_node = NULL;
  mxml_node_t *child_node = NULL;
  mxml_node_t *item_list_node = NULL;
  mxml_node_t *item_node = NULL;
  uuid_t uuid;
  char uuid_str[37];
  FILE *fp = NULL;
  ABFolder *folder;

  uuid_generate(uuid);
  uuid_unparse(uuid, uuid_str);

  folder = addrbook_folder_create();
  if (folder == NULL) {
    return 1;
  }
  ((ABItem *)folder)->uid = uuid_str;
  ((ABItem *)folder)->name = "NewFolder01";

  xml = mxmlNewXML("1.0");

  abook = mxmlNewElement(xml, "addrbook");

  folder_node = mxmlNewElement(abook, ELEM_FOLDER);
  mxmlElementSetAttr(folder_node, ATTR_UID, ((ABItem *)folder)->uid);
  mxmlElementSetAttr(folder_node, ATTR_NAME, ((ABItem *)folder)->name);

  uuid_generate(uuid);
  uuid_unparse(uuid, uuid_str);

  child_node = mxmlNewElement(abook, ELEM_FOLDER);
  mxmlElementSetAttr(child_node, ATTR_UID, uuid_str);
  mxmlElementSetAttr(child_node, ATTR_NAME, "subfolder");

  item_list_node = mxmlNewElement(folder_node, ELEM_ITEM_LIST);
  item_node = mxmlNewElement(item_list_node, ELEM_ITEM);
  mxmlElementSetAttr(item_node, ATTR_TYPE, ATTR_TYPE_VAL_FOLDER);
  mxmlElementSetAttr(item_node, ATTR_UID, uuid_str);

  fp = fopen("addrbook-02.xml", "w");
  if (fp == NULL) {
    return -1;
  }

  mxmlSaveFile(xml, fp, MXML_NO_CALLBACK);

  fclose(fp);

  addrbook_folder_destroy(folder);

  mxmlDelete(item_node);
  mxmlDelete(item_list_node);
  mxmlDelete(child_node);
  mxmlDelete(folder_node);
  mxmlDelete(xml);

  return 0;
}
