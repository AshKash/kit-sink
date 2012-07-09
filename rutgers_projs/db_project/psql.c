/* This transforms a 1D array with colored points to a 2D array.
 * The transformation basically converts the instance of the
 * generalized counting/reporting problem to a instance of the
 * standard counting/reporting problem.
 * See Gupta, Janardhan and Smid paper in Algorithms journal 19
 */

/* This version uses the postgresql database as the backend.
 * One of the cool features of postgres is that it has special
 * datatypes for points and other geometric opbjects.
 * This is used to represent the data in the transformed table,
 * rather than having two fields to represent x,y. */

/* Please ignore the operations on the penta table */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <pgsql/libpq-fe.h>
#include <sys/time.h>

#define MAX_PTS    1000000
#define TOTAL_PTS  100000
#define MIN_P (5)
#define MAX_P (1000000)
#define GROUND (0)
#define N_ITER 3
#define TABLE_INIT
#define BIGTEXT2 "'Indexes are used to find rows with a specific value of one columnfast. Without an index MySQL has to start with the first recordand then read through the whole table until it finds the relevantrows. The bigger the table, the more this costs.'"
#define BIGTEXT "'Hello World'"
#define NUM_COLORS 100000

typedef long long int int64;
typedef unsigned long long int uint64;

/* stores 1D data - color and the point p */
struct gen {
  int64 x[MAX_PTS];
  int color[MAX_PTS];
  int total;
} gen_pts;

/* stores 2D data - (p, pred(p)) and the color */
struct std {
  int64 x[MAX_PTS],
    y[MAX_PTS];
  int color[MAX_PTS];
  unsigned int total;
} std_pts;

/* stores the x^5 + y^5 */
struct penta {
  int64 x5[MAX_PTS];
  int64 x[MAX_PTS];
  int color[MAX_PTS];
  unsigned int total;
} penta_pts;

/* The connection struct */
PGconn *dbase;

/* Init the db and connect */
int dbinit(PGconn **dbase)
{

    char       *pghost,
               *pgport,
               *pgoptions,
               *pgtty;
    char       *dbName;

    /*
     * begin, by setting the parameters for a backend connection if the
     * parameters are null, then the system will try to use reasonable
     * defaults by looking up environment variables or, failing that,
     * using hardwired constants
     */
    pghost = NULL;              /* host name of the backend server */
    pgport = NULL;              /* port of the backend server */
    pgoptions = NULL;           /* special options to start up the backend
                                 * server */
    pgtty = NULL;               /* debugging tty for the backend server */
    dbName = "2d";

    /* make a connection to the database */
    *dbase = PQsetdb(pghost, pgport, pgoptions, pgtty, dbName);

    /*
     * check to see that the backend connection was successfully made
     */
    if (PQstatus(*dbase) == CONNECTION_BAD)
    {
        fprintf(stderr, "Connection to database '%s' failed.\n", dbName);
        fprintf(stderr, "%s", PQerrorMessage(*dbase));
        return -1;
    }
    return 1;
}

/* sends the query to the db, takes printfstyle args */
inline int dbquery(PGconn *conn, void (*procResult) (PGresult *),
		   const char *format, ...)
{
  char sqlbuff[4096];
  va_list ap;
  PGresult *res;
  int s;

  /*
   * now construct SQL query in 'sqlbuff' 
   */
  
  va_start (ap, format);
  vsprintf(sqlbuff, format, ap);
  va_end(ap);
  //printf("sending %s\n", sqlbuff);

  /*
   *   fetch rows from the pg_database, the system catalog of
   * databases
   */
  res = PQexec(conn, sqlbuff);
  s = PQresultStatus(res);
  if (!res || !(s == PGRES_COMMAND_OK || s == PGRES_TUPLES_OK)) {
    fprintf(stderr, "Query: '%s'\n", sqlbuff);
    fprintf(stderr, "%s: %s", PQresStatus(s),
	    PQresultErrorMessage(res));
    PQclear(res);
    return -1;
  }

  /* call the func to process results */
  if (procResult)
    procResult(res);
  PQclear(res);
  
  /*
    if (mysql_real_query(dbase, sqlbuff, strlen(sqlbuff))) {
    printf("%s\n", mysql_error(dbase));
    return -1;
    }
  */
  return 1;
}

/* sends the query to the db, takes printfstyle args */
inline int dbquery2(PGconn *conn, void (*procResult) (PGresult *),
		   const char *format, ...)
{
  char sqlbuff[4096], sqlbuff2[4096];
  va_list ap;
  PGresult *res;

  /*
   * now construct SQL query in 'sqlbuff' 
   */
  
  va_start (ap, format);
  sprintf(sqlbuff2, "DECLARE mycursor CURSOR FOR %s", format);
  vsprintf(sqlbuff, sqlbuff2, ap);
  va_end(ap);
  //printf("sending %s\n", sqlbuff);

  /* start a transaction block */
  res = PQexec(conn, "BEGIN");
  if (!res || PQresultStatus(res) != PGRES_COMMAND_OK) {
    fprintf(stderr, "BEGIN command failed\n");
    PQclear(res);
    return -1;
  }
  PQclear(res);

  /*
   * fetch rows from the pg_database, the system catalog of
   * databases
   */
  res = PQexec(conn, sqlbuff);
  if (!res || PQresultStatus(res) != PGRES_COMMAND_OK) {
    fprintf(stderr, "Query: '%s'\n", sqlbuff);
    fprintf(stderr, "Reason: %s", PQresultErrorMessage(res));
    PQclear(res);
    return -1;
  }
  PQclear(res);
  res = PQexec(conn, "FETCH ALL in mycursor");
  if (!res || PQresultStatus(res) != PGRES_TUPLES_OK) {
    fprintf(stderr, "FETCH ALL command didn't return tuples properly\n");
    fprintf(stderr, "Reason: %s", PQresultErrorMessage(res));
    PQclear(res);
    return -1;
  }
  
  /* call the func to process results */
  if (procResult)
    procResult(res);
  PQclear(res);
  
  /* close the cursor */
  res = PQexec(conn, "CLOSE mycursor");
  PQclear(res);
  
  /* commit the transaction */
    res = PQexec(conn, "COMMIT");
    PQclear(res);

  /*
    if (mysql_real_query(dbase, sqlbuff, strlen(sqlbuff))) {
    printf("%s\n", mysql_error(dbase));
    return -1;
    }
  */
  return 1;
}

/* creates tables with proper schema */
int table_init(PGconn *dbase)
{
  int r;

  /* drop tables gen, std and penta */
  printf("Table init\n");
  dbquery(dbase, NULL, "drop table gen");
  dbquery(dbase, NULL, "drop table penta");
  dbquery(dbase, NULL, "drop table std");

  /* create with proper schema */
  r = dbquery(dbase, NULL, "create table gen (color int, x bigint, "
	      "t1 text, t2 text, t3 text, t4 text, t5 text)");
  if (r < 0) return r;

  r = dbquery(dbase, NULL, "create table std "
	      "(color int, coord point, "
	      "t1 text, t2 text, t3 text, t4 text, t5 text)");
  if (r < 0) return r;

  r = dbquery(dbase, NULL, "create table penta "
	      "(color int, x5 bigint, x bigint, "
	      "t1 text, t2 text, t3 text, t4 text, t5 text)");
  return r;

  /* create proper indices */
  r = dbquery(dbase, NULL, "create index genidx on gen using btree (x)");
  if (r < 0) return r;

  r = dbquery(dbase, NULL, "create index stdidx on std using "
	      "(box(coord coord)");
  if (r < 0) return r;

  r = dbquery(dbase, NULL, "create index genidx on penta (x, x5)");
  if (r < 0) return r;

}

/* prints results in a tabular form */
void printres(PGresult *result) {
  int nFields, nRows, i, j;

  /* first, print out the attribute names */
  nFields = PQnfields(result);
  nRows = PQntuples(result);
  printf("Total of %d results found\n", nRows);
    
  /* next, print out the rows */
  /*
  for (i = 0; i < nRows; i++) {
    for (j = 0; j < nFields; j++)
      printf("%s\t", PQgetvalue(result, i, j));
  }
  
  printf("\n"); 
  */
}

/* adds a random set of points to the gen_pts struct
 * returns the total */
inline void add_gen_pts()
{

  int i, r1, r2, color;
  int64 x;

  /* init */
  gen_pts.total = 0;
  srand(TOTAL_PTS);

  // This is modified to skew the data 
  // so we have only on color from 0 to TOTAL_PTS - NUM_COLORS
  // then the rest in succession
  for (i=0; i< TOTAL_PTS; i++) {
    r1 = rand();
    r2 = rand();

    if (gen_pts.total > MAX_PTS) {
      printf("Error, table is full\n");
      exit(1);
    }

    /* the point should fall b/w MIN_P and MAX_P */
    x = (double) MAX_P * r1/RAND_MAX + MIN_P;
    color = NUM_COLORS * (double)r2/RAND_MAX;

    gen_pts.x[gen_pts.total] = x;
    gen_pts.color[gen_pts.total] = color;
    gen_pts.total++;
    
  }
}

/* returns x^5 */
inline int64 pow5(int64 a)
{
  int64 b = a*a;
  b *= b;
  b *= a;

  return b;
}

/* compares two long longs and retuns -1, 0 or +1 */
inline int compare(const void *a, const void *b) {
  if (*(int64 *)a < *(int64 *)b)
    return -1;
  else if (*(int64 *)a > *(int64 *)b)
    return +1;

  return 0;
}

/* given a set of points in the general form, converts it
 * to the standard form */
/* FIXME: this is not at all optimized */
void gen2std(const struct gen *g, struct std *s)
{
  int total = g->total;
  int i, j, k;
  int color;
  int64 x[MAX_PTS];

  /* for each color, sort the points in gen */
  s->total = 0;
  for (color=0; color < NUM_COLORS; color++) {
    j = 0;
    for (i=0; i<total; i++) {
      if (g->color[i] == color)
	x[j++] = g->x[i];
    }
    /* sort this */
    //printf("Found %d for color %d\n", j, color);
    qsort((void *)x, j, sizeof(*x), compare);

    /* now put this into the std struct */
    // first ele for this color
    if (j) {
      s->color[s->total] = color;
      s->x[s->total] = x[0];
      s->y[s->total] = GROUND;
      s->total++;
    }
    for (k=1; k < j; k++) {
      s->color[s->total] = color;
      s->x[s->total] = x[k];
      s->y[s->total] = x[k-1];
      s->total++;
    }
  }
}

/* Converts the std_pts to x^5 + y^5 */
void std2penta(const struct std *s, struct penta *p)
{
  int i;

  p->total = 0;
  for (i=0; i < s->total; i++) {
    p->x5[i] = pow5(s->x[i]) + pow5(s->y[i]);
    p->x[i] = s->x[i];
    p->color[i] = s->color[i];
    p->total++;
  }
}

/* fills the points from the structure into the gen table
 * returns status of the query execution */
int fill_gen_table(PGconn *dbase, struct gen *g)
{
  int i, r;

  r = dbquery(dbase, NULL, "BEGIN");
  if (r < 0) {
    printf("SQL error\n");
    return r;
  }
  for (i=0; i<g->total; i++) {
    r = dbquery(dbase, NULL, "INSERT INTO gen (color, x, t1, t2, t3, t4, t5) "
		"VALUES (%d, %lld, %s, %s, %s, %s, %s)",
		g->color[i], g->x[i],
		BIGTEXT, BIGTEXT, BIGTEXT, BIGTEXT, BIGTEXT);
    if (r < 0) {
      printf("SQL error\n");
      return r;
    }
  }
  r = dbquery(dbase, NULL, "COMMIT");
  if (r < 0) {
    printf("SQL error\n");
    return r;
  }


  return r;
}

/* fills the penta table and returns query status */
int fill_penta_table(PGconn *dbase, struct gen *g)
{
  int i, r;

  /* first fill the std struct, then the penta struct */
  gen2std(g, &std_pts);
  std2penta(&std_pts, &penta_pts);

  /* now put the penta struct to penta table */
  r = dbquery(dbase, NULL, "BEGIN");
  if (r < 0) {
    printf("SQL error\n");
    return r;
  }
  for (i=0; i<penta_pts.total; i++) {
    r = dbquery(dbase, NULL, "INSERT INTO penta (color, x5, x, t, t2, t3, t4, t5) " 
		"VALUES (%d, %lld, %lld, %s, %s, %s, %s, %s)",
		penta_pts.color[i], penta_pts.x5[i],
		penta_pts.x[i],
		BIGTEXT, BIGTEXT, BIGTEXT, BIGTEXT, BIGTEXT);
    if (r < 0) {
      printf("SQL error\n");
      return r;
    }
  }
  r = dbquery(dbase, NULL, "COMMIT");
  if (r < 0) {
    printf("SQL error\n");
    return r;
  }

  return r; 
}

/* fills the std table and returns query status */
/* This version uses the postgresql database as the backend.
 * One of the cool features of postgres is that it has special
 * datatypes for points and other geometric opbjects.
 * This is used to represent the data in the transformed table,
 * rather than having two fields to represent x,y.
 */
int fill_std_table(PGconn *dbase, struct gen *g)
{
  int i, r;

  /* fill the std struct */
  gen2std(g, &std_pts);

  /* now put the std struct to std table */
  r = dbquery(dbase, NULL, "BEGIN");
  if (r < 0) {
    printf("SQL error\n");
    return r;
  }
  for (i=0; i<std_pts.total; i++) {
    r = dbquery(dbase, NULL, "INSERT INTO std "
		"(color, coord, t1, t2, t3, t4, t5) " 
		"VALUES (%d, '(%lld, %lld)', %s, %s, %s, %s, %s)",
		std_pts.color[i], std_pts.x[i],
		std_pts.y[i],
		BIGTEXT, BIGTEXT, BIGTEXT, BIGTEXT, BIGTEXT);
    if (r < 0) {
      printf("SQL error\n");
      return r;
    }
  }
  r = dbquery(dbase, NULL, "COMMIT");
  if (r < 0) {
    printf("SQL error\n");
    return r;
  }

  return r; 
}

/* inserts points to the data base and prints time */
int time_fills(PGconn *dbase)
{
  struct timeval tv1, tv2;
  double t;
  int r;

  /* add the points to gen_pts */
  gen_pts.total = std_pts.total = penta_pts.total = 0;
  add_gen_pts();

  /* Fill these points into the gen table of the DB */
  gettimeofday(&tv1, NULL);
  r = fill_gen_table(dbase, &gen_pts);
  if (r<0) {
    printf("fill_gen_table failed\n");
    return -1;
  }
  gettimeofday(&tv2, NULL);
  t = (tv2.tv_sec - tv1.tv_sec) +
    (double) (tv2.tv_usec - tv1.tv_usec) / 1000000;
  printf("Time taken to fill gen table: %lf sec\n", t);
  // ensures all io is finished
  sleep(5); sync(); sync();


  /* fill these points into the std table */
  gettimeofday(&tv1, NULL);
  r = fill_std_table(dbase, &gen_pts);
  if (r<0) {
    printf("fill_std_table failed\n");
    return -1;
  }
  gettimeofday(&tv2, NULL);
  t = (tv2.tv_sec - tv1.tv_sec) + 
    (double) (tv2.tv_usec - tv1.tv_usec) / 1000000;
  printf("Time taken to fill std table: %lf sec\n", t);
  // ensures all io is finished
  sleep(5); sync(); sync();

#ifdef FALSE
  /* fill these points into the penta table */
  gettimeofday(&tv1, NULL);
  r = fill_penta_table(dbase, &gen_pts);
  if (r<0) {
    printf("fill_penta_table failed\n");
    return -1;
  }
  gettimeofday(&tv2, NULL);
  t = (tv2.tv_sec - tv1.tv_sec) + 
    (double) (tv2.tv_usec - tv1.tv_usec) / 1000000;
  printf("Time taken to fill penta table: %lf sec\n", t);
  // ensures all io is finished
  sleep(5); sync(); sync();

#endif

  return 1;

}

/* times the time to execute the queries on the two tables */
int time_query(PGconn *dbase, int64 start, int64 end)
{
  struct timeval tv1, tv2;
  double t, tavg;
  int r, i;
  int64 starty, start5, endy, end5;
  
  /* the query for the gen table must use group by
   * on the other hand all queries on penta must give
   * results that are already free of duplicates.
   * So in order to emphasize the duplicate removal,
   * the queries will span the entire range */
  printf("Asking for all colors of x between %lld and %lld\n",
	 start, end);
  printf("Data range: %d - %d\n", MIN_P, MAX_P);

  /*** Do for the gen table ***/
  printf("Time taken to QUERY gen table: ");
  tavg = 0;
  for (i=0; i < N_ITER; i++) {

    gettimeofday(&tv1, NULL);
    r = dbquery(dbase, printres, "SELECT color FROM gen WHERE " 
		"x>=%lld AND x<%lld GROUP BY color",
		start, end);
    if (r<0) {
      printf("Query on gen table failed\n");
      return -1;
    }
    gettimeofday(&tv2, NULL);
    t = (tv2.tv_sec - tv1.tv_sec) +
      (double) (tv2.tv_usec - tv1.tv_usec) / 1000000;
    tavg += t;
    printf("%lf\t", t);
  }
  printf("Avg: %lf\n", tavg/3);

  /*** Do for the std table ***/
  printf("Time taken to QUERY std table: ");
  tavg = 0;
  for (i=0; i< N_ITER; i++) {
    gettimeofday(&tv1, NULL);
    /* (l,r) -> [l,r] x [-oo, l]
     * in our case, -oo is really GROUND */
    starty = GROUND;
    endy = start;
    
    r = dbquery(dbase, printres, "SELECT color FROM std WHERE " 
		"coord @ box '((%lld, %lld), (%lld, %lld))' GROUP BY color",
		start, starty,
		end, endy
		);
    
    /*  r = dbquery(dbase, printres, "SELECT color FROM std WHERE "
	"coord >> '(%lld, 0)' AND coord << '(%lld,0)' AND "
	"coord >^ '(0,%lld)' AND coord <^ '(0,%lld)'",
	start, end, starty, endy
	);*/
    if (r<0) {
      printf("Query on std table failed\n");
      return -1;
    }
    gettimeofday(&tv2, NULL);
    t = (tv2.tv_sec - tv1.tv_sec) + 
      (double) (tv2.tv_usec - tv1.tv_usec) / 1000000;
    tavg += t;
    printf("%lf\t", t);
  }
  printf("Avg: %lf\n", tavg/3);

  /*** Do for the penta table ***/
  gettimeofday(&tv1, NULL);
  /* (l,r) -> [l,r] x [-oo, l] -> (l^5+ -oo^5, l^5 + r^5)
   * in our case, -oo is really GROUND */
  start5 = pow5(start) + pow5(starty);
  end5 = pow5(end) + pow5(endy);
  r = dbquery(dbase, NULL, "SELECT color FROM penta WHERE x5>=%lld AND x5<%lld",
	      start5, end5);
  if (r<0) {
    printf("Query on penta table failed\n");
    return -1;
  }
  gettimeofday(&tv2, NULL);
  t = (tv2.tv_sec - tv1.tv_sec) + 
    (double) (tv2.tv_usec - tv1.tv_usec) / 1000000;
  printf("Time taken to QUERY penta table: %lf sec\n", t);

  return 1;
}

int nice_exit()
{
  PQfinish(dbase);
  exit(1);
}

/*
 * First add a set of points to the gen_pts struct.
 *
 * Then these points are added into the gen table in the
 * database. The operation is timed.
 * We then convert the gen_pts to penta_pts and add these
 * points to the penta table in the DB. This is also timed.
 *
 * The second part is to query the two tables and time how
 * long it takes to complete each query
 */
int main(int argc, char **argv)
{
  int r, i;

  atexit(nice_exit);
  /* connect to DB */
  if (dbinit(&dbase) < 0) {
    printf("Error connecting to database\n");
    return -1;
  }

#ifdef TABLE_INIT
  /* table init */
  r = table_init(dbase);
  if (r < 0) {
    printf("table_init failed\n");
    return -1;
  }

  /* time the adding of points */
  if (time_fills(dbase) < 0) {
    return -1;
  }
#endif

  /* time the queries */
  if (time_query(dbase, MIN_P, MAX_P) < 0) {
    return -1;
  }

  return 0;  
}
