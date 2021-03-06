#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>

/**
 * https://tools.ietf.org/html/rfc1035
 *
 * 4.1.1. Header section format
 *
 * The header contains the following fields:
 *
 *                                   1  1  1  1  1  1
 *     0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
 *  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *  |                      ID                       |
 *  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *  |QR|   Opcode  |AA|TC|RD|RA|   Z    |   RCODE   |
 *  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *  |                    QDCOUNT                    |
 *  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *  |                    ANCOUNT                    |
 *  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *  |                    NSCOUNT                    |
 *  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *  |                    ARCOUNT                    |
 *  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 */
typedef struct dns_header_s {
  uint16_t id;
  uint16_t flags;
  uint16_t qdcount;
  uint16_t ancount;
  uint16_t nscount;
  uint16_t arcount;
} dns_header_t;

static int check_ids_match(uint16_t query_id, uint16_t reply_id)
{
  if (reply_id != query_id) {
    return 0;
  }

  return 1;
}

#define FLAG_QR     /* 1000 0000 0000 0000 */ 0x8000U /* Query (0) or Response (1) bit field */
#define OPCODE_MASK /* 0111 1000 0000 0000 */ 0x7800U
#define FLAG_AA     /* 0000 0100 0000 0000 */ 0x0400U /* Authoritative Answer - server flag */
#define FLAG_TC     /* 0000 0010 0000 0000 */ 0x0200U /* TrunCated - server flag */
#define FLAG_RD     /* 0000 0001 0000 0000 */ 0x0100U /* Recursion Desired - query flag */
#define FLAG_RA     /* 0000 0000 1000 0000 */ 0x0080U /* Recursion Available - server flag */
#define FLAG_Z      /* 0000 0000 0100 0000 */ 0x0040U /* Reserved for future use. Must be zero */
#define RCODE_MASK  /* 0000 0000 0000 1111 */ 0x000fU /* Response Code - 4 bit field */
#define FLAG_AD     /* 0000 0000 0010 0000 */ 0x0020U /* Authenticated Data - server flag */
#define FLAG_CD     /* 0000 0000 0001 0000 */ 0x0010U /* Checking Disabled - query flag */

#define OPCODE_SHIFT 11

/* This type represents a domain name in wire format. */
typedef uint8_t dns_dname_t;

typedef struct dns_query_s {
  dns_dname_t *qname;
  size_t qnamelen;
  uint16_t qtype;
  uint16_t qclass;
} dns_query_t;

/* Resource Record definitions. */

/**
 * Resource record class codes.
 *
 * http://www.iana.org/assignments/dns-parameters/dns-parameters.xhtml
 */
enum dns_rr_class {
  DNS_RR_CLASS_IN   = 1,   /* Internet */
  DNS_RR_CLASS_CH   = 3,   /* Chaos */
  DNS_RR_CLASS_NONE = 254, /* QCLASS NONE */
  DNS_RR_CLASS_ANY  = 255  /* QCLASS * (ANY) */
};

/**
 * Resource record types.
 *
 * http://www.iana.org/assignments/dns-parameters/dns-parameters.xhtml
 */
enum dns_rr_type {
  DNS_RR_TYPE_A = 1, /* An IPv4 host address. */
};

static const char *dns_rr_class_names[] = {
  [DNS_RR_CLASS_IN]   = "IN",
  [DNS_RR_CLASS_CH]   = "CH",
  [DNS_RR_CLASS_NONE] = "NONE",
  [DNS_RR_CLASS_ANY]  = "ANY"
};

#define DNS_DEFAULT_PORT "53"

/*
 * RFC 1035, section 4.2.1: Messages carried by UDP are restricted to 512
 * bytes (not counting the IP nor UDP headers).
 */
#define MAX_UDP_SIZE 512

#define MAX_PACKET_LEN 65535

/*
 * Basic limits for domain names (RFC 1035).
 */
#define DNS_DNAME_MAXLEN 255 /* 1-byte maximum. */
#define DNS_DNAME_MAXLABELLEN 63 /* 2^6 - 1 */

/**
 * DNS operation codes (OPCODEs).
 */
enum dns_opcode {
  DNS_OPCODE_QUERY = 0,  /* Standard query. */
  DNS_OPCODE_IQUERY = 1, /* Inverse query. */
  DNS_OPCODE_STATUS = 2, /* Server status request */
  DNS_OPCODE_NOTIFY = 4, /* Notify message. */
  DNS_OPCODE_UPDATE = 5  /* Dynamic update. */
};

/**
 * DNS reply codes (RCODEs).
 */
enum dns_rcode {
  DNS_RCODE_NOERROR = 0, /* No error. */
  DNS_RCODE_FORMERR = 1, /* Format error. */
  DNS_RCODE_SERVFAIL = 2, /* Server failure. */
  DNS_RCODE_NXDOMAIN = 3, /* Non-existent domain. */
  DNS_RCODE_NOTIMPL = 4, /* Not implemented. */
  DNS_RCODE_REFUSED = 5, /* Refused. */
  DNS_RCODE_YXDOMAIN = 6, /* Name should not exist. */
  DNS_RCODE_YXRRSET = 7, /* RR set should not exist. */
  DNS_RCODE_NXRRSET = 8, /* RR set does not exist. */
  DNS_RCODE_NOTAUTH = 9, /* Server not authoritative / Query not authorized. */
  DNS_RCODE_NOTZONE = 10 /* Name is not inside zone. */
};

typedef struct {
  uint16_t len;
  uint8_t data[];
} rdata_t;

#define DNS_WIRE_HEADER_SIZE 12

/**
 * A generic entry that can be used to build a lookup table.
 */
typedef struct {
  int id;
  const char *name;
} lookup_entry_t;

/**
 * Looks up the given id in the lookup table.
 *
 * @param[in]   table   Lookup table.
 * @param[in]   id      ID to look up.
 *
 * @returns
 */
static const lookup_entry_t *lookup_by_id(const lookup_entry_t *table, int id) {
  while (table->name != NULL) {
    if (table->id == id) {
      return table;
    }
    table++;
  }

  return NULL;
}

/**
 * DNS reply code names.
 */
static const lookup_entry_t rcode_names[] = {
  { DNS_RCODE_NOERROR, "NOERROR" },
  { DNS_RCODE_FORMERR, "FORMERR" },
  { DNS_RCODE_SERVFAIL, "SERVFAIL" },
  { DNS_RCODE_NXDOMAIN, "NXDOMAIN" },
  { DNS_RCODE_NOTIMPL, "NOTIMPL" },
  { DNS_RCODE_REFUSED, "REFUSED" },
  { DNS_RCODE_YXDOMAIN, "YXDOMAIN" },
  { DNS_RCODE_YXRRSET, "YXRRSET" },
  { DNS_RCODE_NXRRSET, "NXRRSET" },
  { DNS_RCODE_NOTAUTH, "NOTAUTH" },
  { DNS_RCODE_NOTZONE, "NOTZONE" },
  { 0, NULL }
};

/**
 * DNS operation code names.
 */
static const lookup_entry_t opcode_names[] = {
  { DNS_OPCODE_QUERY, "QUERY" },
  { DNS_OPCODE_IQUERY, "IQUERY" },
  { DNS_OPCODE_STATUS, "STATUS" },
  { DNS_OPCODE_NOTIFY, "NOTIFY" },
  { DNS_OPCODE_UPDATE, "UPDATE" },
  { 0, NULL }
};

static void write_uint16(void *dst, uint16_t data) {
  uint8_t *p = (uint8_t *)dst;
  p[0] = (uint8_t)((data >> 8) & 0xFF);
  p[1] = (uint8_t)(data & 0xFF);
}

/**
 * Reads to 2 bytes from |src|.
 *
 * Returns the 2 bytes read, in the host byte order.
 */
static uint16_t read_uint16(const void *src) {
  const uint8_t *p = (const uint8_t *)src;
  return ((uint16_t)p[0] << 8) | (uint16_t)p[1];
}

#define DNS_OFFSET_ID 0
#define DNS_OFFSET_FLAGS 2
#define DNS_OFFSET_QDCOUNT 4
#define DNS_OFFSET_ANCOUNT 6
#define DNS_OFFSET_NSCOUNT 8
#define DNS_OFFSET_ARCOUNT 10

static void dns_pkt_set_id(uint8_t *packet, uint16_t id)
{
  write_uint16(packet + DNS_OFFSET_ID, id);
}

static uint16_t dns_pkt_get_id(const uint8_t *packet)
{
  return read_uint16(packet + DNS_OFFSET_ID);
}

static void dns_pkt_set_qdcount(uint8_t *packet, uint16_t qdcount)
{
  write_uint16(packet + DNS_OFFSET_QDCOUNT, qdcount);
}

static uint16_t dns_pkt_get_qdcount(uint8_t *packet)
{
  return read_uint16(packet + DNS_OFFSET_QDCOUNT);
}

static void dns_pkt_set_ancount(uint8_t *packet, uint16_t ancount)
{
  write_uint16(packet + DNS_OFFSET_ANCOUNT, ancount);
}

static uint16_t dns_pkt_get_ancount(uint8_t *packet)
{
  return read_uint16(packet + DNS_OFFSET_ANCOUNT);
}

static void dns_pkt_set_nscount(uint8_t *packet, uint16_t nscount)
{
  write_uint16(packet + DNS_OFFSET_NSCOUNT, nscount);
}

static uint16_t dns_pkt_get_nscount(uint8_t *packet)
{
  return read_uint16(packet + DNS_OFFSET_NSCOUNT);
}

static void dns_pkt_set_arcount(uint8_t *packet, uint16_t arcount)
{
  write_uint16(packet + DNS_OFFSET_ARCOUNT, arcount);
}

static uint16_t dns_pkt_get_arcount(uint8_t *packet)
{
  return read_uint16(packet + DNS_OFFSET_ARCOUNT);
}

static int is_root(const char *name) {
  return name[0] == '.' && name[1] == '\0';
}

static size_t domain_name_length(const char *name) {
  size_t length = strlen(name);

  while (length > 0 && name[length - 1] == '.') {
    length -= 1;
  }

  if (length + 1 > DNS_DNAME_MAXLEN) {
    return 0;
  }

  return length;
}

/*
 * Code from knot: src/dnssec/shared/dname.{c.h}:dname_from_ascii
 */
static uint8_t *dname_from_str(const char *name) {
  if (!name) {
    return NULL;
  }

  if (is_root(name)) {
    return (uint8_t *)strdup("");
  }

  size_t length = domain_name_length(name);
  if (length == 0) {
    return NULL;
  }

  char *wire = strndup(name, length);
  if (!wire) {
    return NULL;
  }

  uint8_t *dname = malloc(length + 2);
  if (!dname) {
    return NULL;
  }

  uint8_t* write = dname;
  char *save = NULL;
  char *label = strtok_r(wire, ".", &save);
  while (label) {
    size_t label_len = strlen(label);
    if (label_len > DNS_DNAME_MAXLABELLEN) {
      free(dname);
      return NULL;
    }
    *write = (uint8_t)label_len;
    write += 1;

    memcpy(write, label, label_len);
    write += label_len;

    label = strtok_r(NULL, ".", &save);
  }

  *write = '\0';

  return dname;
}

/*
 * Code from knot: src/contrib/sockaddr.{c.h}:sockaddr_tostr
 */

/* Address string "address[@port]" maximum length. */
#define SOCKADDR_STRLEN_EXT (1 + 6) /* '@', 5 digits number, \0 */
#define SOCKADDR_STRLEN (sizeof(struct sockaddr_un) + SOCKADDR_STRLEN_EXT)

#define NET_ERR -1

/*
 * Returns the port number from address.
 *
 * \param sa Socket address.
 *
 * \return The port number or error code.
 */
static int sockaddr_port(const struct sockaddr *sa) {
  if (sa == NULL) {
    return NET_ERR;
  }

  if (sa->sa_family == AF_INET6) {
    return ntohs(((struct sockaddr_in6 *)sa)->sin6_port);
  } else if (sa->sa_family == AF_INET) {
    return ntohs(((struct sockaddr_in *)sa)->sin_port);
  } else {
    return NET_ERR;
  }
}

/*
 * Returns the string representation of socket address.
 *
 * \note String format: <address>[@<port>], f.e. '127.0.0.1@53'
 *
 * \param buf Destination for the string representation.
 * \param maxlen Maximum number of written bytes.
 * \param sa Socket address.
 *
 * \return On success returns the number of bytes writte, otherwise an error code.
 */
static int sockaddr_tostr(char *buf, size_t maxlen, const struct sockaddr *sa) {
  if (sa == NULL || buf == NULL) {
    return NET_ERR;
  }

  const char *out = NULL;

  if (sa->sa_family == AF_INET6) {
    const struct sockaddr_in6 *s = (const struct sockaddr_in6 *)sa;
    out = inet_ntop(sa->sa_family, &s->sin6_addr, buf, maxlen);
  } else if (sa->sa_family == AF_INET) {
    const struct sockaddr_in *s = (const struct sockaddr_in *)sa;
    out = inet_ntop(sa->sa_family, &s->sin_addr, buf, maxlen);
  } else if (sa->sa_family == AF_UNIX) {
    /*    const struct sockaddr_un *s =  (const struct sockaddr_un *)sa;
     *    size_t ret = strlcpy(buf, s->sun_path, maxlen);
     *    out = (ret < maxlen) ? buf : NULL;
     */
  } else {
    return NET_ERR;
  }

  if (out == NULL) {
    *buf = '\0';
    return -2;
  }

  /* Write separator and port. */
  int written = strlen(buf);
  int port = sockaddr_port(sa);
  if (port > 0) {
    int ret = snprintf(&buf[written], maxlen - written, "@%d", port);
    if (ret <= 0 || (size_t) ret >= maxlen - written) {
      *buf = '\0';
      return -2;
    }

    written += ret;
  }

  return written;
}

#define DNS_LABEL_PTR 0xC0 /* 1100 0000 */

static int is_label_pointer(const uint8_t *pos) {
  return pos && ((pos[0] & DNS_LABEL_PTR) == DNS_LABEL_PTR);
}

/*
 * Code from knot src/libknot/dname.{c,h}:knot_dname_size().
 */
static int dns_dname_size(const uint8_t *name) {
  int len = 0;

  if (name == NULL) {
    return -1;
  }

  /* Count name length without terminal label. */
  while (*name != '\0' && !is_label_pointer(name)) {
    uint8_t label_len = *name + 1;
    len += label_len;
    name += label_len;
  }

  /* Compression pointer is 2 octets. */
  if (is_label_pointer(name)) {
    return len + 2;
  }

  return len + 1;
}

int main(int argc, char **argv) {
  char *owner;
  struct dns_query_s *question;
  uint8_t *query_pkt;
  size_t query_pktlen;
  char *server_name;
  char *def_port;
  struct addrinfo hints, *addrlist;
  int rv;
  int sockfd;
  uint8_t reply_pkt[MAX_PACKET_LEN];
  struct sockaddr_storage from;
  socklen_t fromlen = sizeof(from);
  ssize_t reply_pktlen;
  struct dns_header_s *reply_header;

  if (argc != 2) {
    fprintf(stderr, "usage: %s name\n", argv[0]);
    return EXIT_FAILURE;
  }

  owner = strdup(argv[1]);

  /* Prepare the packet (header + question) with the query that will be sent to
   * the DNS server.
   */
  query_pkt = malloc(MAX_UDP_SIZE);

  /* Create QNAME from string. */
  dns_dname_t *qname = dname_from_str(owner);

  question = malloc(sizeof(*question));
  memset(question, 0, sizeof(*question));

  question->qname = qname;
  question->qnamelen = dns_dname_size(qname);
  question->qtype = DNS_RR_TYPE_A;
  question->qclass = DNS_RR_CLASS_IN;

  unsigned int rand_seed;
  struct timeval tv;
  gettimeofday(&tv, NULL);
  rand_seed = tv.tv_sec ^ tv.tv_usec;
  srandom(rand_seed);

  uint8_t *position;

  /* HEADER */
  dns_pkt_set_id(query_pkt, random());
  write_uint16(query_pkt + DNS_OFFSET_FLAGS, FLAG_RD);
  dns_pkt_set_qdcount(query_pkt, 1);

  position = query_pkt + DNS_WIRE_HEADER_SIZE;

  /* QUESTION: QNAME + QTYPE + QCLASS */

  /* QNAME */
  memcpy(position, question->qname, question->qnamelen);
  position += question->qnamelen;

  /* QTYPE */
  write_uint16(position, question->qtype);
  position += sizeof(question->qtype);

  /* QCLASS */
  write_uint16(position, question->qclass);
  position += sizeof(question->qclass);

  query_pktlen = position - query_pkt;
  /*  printf("PKTLEN: %zu\n", query_pktlen); */

  /* Resolver settings. */
  server_name = "8.8.8.8";
  def_port = DNS_DEFAULT_PORT;

  /* Prepare the UDP socket that will be used to send the query to the DNS
   * server.
   */
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;

  if ((rv = getaddrinfo(server_name, def_port, &hints, &addrlist)) != 0) {
    fprintf(stderr, "getaddrinfo failed: %s\n", gai_strerror(rv));
    return EXIT_FAILURE;
  }

  if (addrlist == NULL) {
    fprintf(stderr, "no addrlist");
    return EXIT_FAILURE;
  }

  char addr_str[SOCKADDR_STRLEN] = { 0 };
  sockaddr_tostr(addr_str, sizeof(addr_str), addrlist->ai_addr);

  /* Create the socket. */
  if ((sockfd = socket(addrlist->ai_family, addrlist->ai_socktype, 0)) == -1) {
    fprintf(stderr, "socket creation failed: %s\n", strerror(errno));
    return EXIT_FAILURE;
  }

  /* Send the query. */
  if (sendto(sockfd, query_pkt, query_pktlen, 0, addrlist->ai_addr,
             addrlist->ai_addrlen) != (ssize_t)query_pktlen) {
    fprintf(stderr, "can't send query to %s\n", addr_str);
    close(sockfd);
    return EXIT_FAILURE;
  }

  /* Receive the response. */
  reply_pktlen = recvfrom(sockfd, reply_pkt, sizeof(reply_pkt), 0,
                          (struct sockaddr *)&from, &fromlen);
  /* recvfrom() can also return 0. */
  if (reply_pktlen == -1 || reply_pktlen == 0) {
    fprintf(stderr, "recvfrom failed\n");
    close(sockfd);
    return EXIT_FAILURE;
  }

  freeaddrinfo(addrlist);

  /* Parse reply to the dns_header_s structure. */
  reply_header = malloc(sizeof(*reply_header));
  memset(reply_header, 0, sizeof(*reply_header));

  reply_header->id = dns_pkt_get_id(reply_pkt);

  if (!check_ids_match(dns_pkt_get_id(query_pkt), dns_pkt_get_id(reply_pkt))) {
    fprintf(stderr, "reply ID (%u) is different from query ID (%u)\n", reply_header->id, dns_pkt_get_id(query_pkt));
    return -1;
  }

  reply_header->flags = read_uint16(reply_pkt + DNS_OFFSET_FLAGS);
  reply_header->qdcount = dns_pkt_get_qdcount(reply_pkt);
  reply_header->ancount = dns_pkt_get_ancount(reply_pkt);
  reply_header->nscount = dns_pkt_get_nscount(reply_pkt);
  reply_header->arcount = dns_pkt_get_arcount(reply_pkt);

  uint16_t opcode_id = (reply_header->flags & OPCODE_MASK) >> OPCODE_SHIFT;
  const lookup_entry_t *opcode = lookup_by_id(opcode_names, opcode_id);

  uint16_t rcode_id = reply_header->flags & RCODE_MASK;
  const lookup_entry_t *rcode = lookup_by_id(rcode_names, rcode_id);

  printf(";; ->>HEADER<<- ");

  if (opcode) {
    printf("opcode: %s, ", opcode->name);
  } else {
    printf("opcode: ?? (%u), ", opcode_id);
  }

  if (rcode) {
    printf("rcode: %s, ", rcode->name);
  } else {
    printf("rcode: ?? (%u), ", rcode_id);
  }

  printf("id: %d\n", reply_header->id);
  printf(";; flags:");
  if ((reply_header->flags & FLAG_QR) != 0) {
    printf(" qr");
  }
  if ((reply_header->flags & FLAG_AA) != 0) {
    printf(" aa");
  }
  if ((reply_header->flags & FLAG_TC) != 0) {
    printf(" tc");
  }
  if ((reply_header->flags & FLAG_RD) != 0) {
    printf(" rd");
  }
  if ((reply_header->flags & FLAG_CD) != 0) {
    printf(" cd");
  }
  if ((reply_header->flags & FLAG_RA) != 0) {
    printf(" ra");
  }
  if ((reply_header->flags & FLAG_AD) != 0) {
    printf(" ad");
  }

  printf("; ");
  printf("QUERY: %u, ", reply_header->qdcount);
  printf("ANSWER: %u, ", reply_header->ancount);
  printf("AUTHORITY: %u, ", reply_header->nscount);
  printf("ADDITIONAL: %u\n", reply_header->arcount);

  printf("\n");

  time_t exec_time;
  struct tm tm;
  char date[64];

  exec_time = time(NULL);
  localtime_r(&exec_time, &tm);
  strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S %Z", &tm);

  printf(";; SERVER: %s\n", addr_str);
  printf(";; WHEN: %s\n", date);
  printf(";; MSG SIZE rcvd: %zd\n", reply_pktlen);

  close(sockfd);

  return 0;
}
