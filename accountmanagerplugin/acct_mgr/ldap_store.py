
from trac.core import *
from trac.config import Option
import crypt, md5, sha, base64

from api import IPasswordStore
import ldap

class LdapAuthStore(Component):
    """This store is basically a SessionAuth store (in db.py) but fetches information 
    from LDAP/AD so that the fullname and email are set automatically. The user's 
    password is also stored in trac's DB but is never used and authentication is
    performed by attempting to do a simple_bind to the LDAP server.
    Please configure all the sections correctly!
    """

    implements(IPasswordStore)
    
    ldap_user = Option('account-manager', 'ldap_user', 
                       doc='LDAP user to use for binding')
    ldap_pass = Option('account-manager', 'ldap_pass', 
                       doc='Password of LDAP user')
    ldap_uri = Option('account-manager', 'ldap_uri', 
                      doc='URI of the LDAP server')
    ldap_basedn = Option('account-manager', 'ldap_basedn', 
                         doc='Base DN used while searching')
    
    ldap_fullname_attr = Option('account-manager', 'ldap_fullname_attr',
                                doc='LDAP attribute that represents a user\'s full name',
                                default='CN')
    ldap_email_attr = Option('account-manager', 'ldap_email_attr',
                             doc='LDAP attribute that represents a user\'s email address',
                             default='mail')
    ldap_filter = Option('account-manager', 'ldap_filter',
                         doc='What to search for, the \%s is replaced with the '
                         'user id of the account being added',
                         default='sAMAccountName=%s')

    def __init__(self):
        pass
    
    def get_users(self):
        """Returns an iterable of the known usernames
        """
        db = self.env.get_db_cnx()
        cursor = db.cursor()
        cursor.execute("SELECT DISTINCT sid FROM session_attribute "
                       "WHERE authenticated=1 AND name='name'")
        for sid, in cursor:
            yield sid
 
    def has_user(self, user):
        db = self.env.get_db_cnx()
        cursor = db.cursor()
        exists = False
        cursor.execute("SELECT COUNT(*) FROM session_attribute "
                   "WHERE authenticated=1 AND name='name' "
                   "AND sid=%s", (user,))
        
        if cursor.fetchone()[0] != 0:
            exists = True
        self.env.log.info('LdapAuthStore: has_user %s, %d' % (user, exists))

        return exists
    
    def set_password(self, username, password):
        """Sets the password for the username.  This should create the user account
        if it doesn't already exist.
        Returns True if a new account was created, False if an existing account
        was updated.
        """
        db = self.env.get_db_cnx()
        cursor = db.cursor()

        # Set name/email from LDAP
        try: 
            (name, email) = self._setuserinfo(username)
        except Exception, e:
            raise TracError('User information not available. Ldap Error: %s' % e)

        cursor.execute("REPLACE INTO session(sid, authenticated, last_visit) "
                       "VALUES(%s, 1, 0)", (username,))
        cursor.execute("REPLACE INTO session_attribute(sid, authenticated, name, value) "
                       "VALUES(%s, 1, 'name', %s)", (username, name))
        cursor.execute("REPLACE INTO session_attribute(sid, authenticated, name, value) "
                       "VALUES(%s, 1, 'email', %s)", (username, email))

        db.commit()
        self.env.log.info('LdapAuthStore: set_password %s, %s, %s' % (username, name, email))
        return True

    # Fetch user name and email from LDAP and fill it in the DB
    def _setuserinfo(self, username):

        attrs = [str(self.ldap_fullname_attr), str(self.ldap_email_attr)]
        ldap_filter = self.ldap_filter % username

        #self.env.log.info('LdapAuthStore: %s, %s, %s, %s' % (self.ldap_fullname_attr, self.ldap_email_attr, ldap_filter, str(attrs)))
        #self.env.log.info('LdapAuthStore: %s' % (self.ldap_basedn))

        con = ldap.initialize(self.ldap_uri)
        con.simple_bind_s(self.ldap_user, self.ldap_pass)

        ldap_users = con.search_s( self.ldap_basedn, ldap.SCOPE_ONELEVEL, 
                                   ldap_filter, attrs )
        if len(ldap_users) == 0:
            try:
                con.unbind_s()
            except:
                pass
            raise TracError('LdapAuthStore: User %s Not found' % username)

        udict = ldap_users[0][1]
        mail = cn = ac = None
        if 'mail' in udict:
            mail = udict['mail'][0].lower()
        if 'cn' in udict:
            cn = udict['cn'][0]

        try:
            con.unbind_s()
        except:
            pass
        return (cn, mail)

    def check_password(self, user, password):
        """Checks if the password is valid for the user.
        Talks to the LDAP server to authenticate given user/password
        """
        valid = False
        try:
            con = ldap.initialize(self.ldap_uri)
            con.simple_bind_s(user, password)
            valid = True
        except ldap.INVALID_CREDENTIALS:
            pass

        finally:
            try:
                con.unbind_s()
            except:
                pass

        if valid:
            return valid

        # see if user exists in db (only for trac_admin)
        if user == 'trac_admin':
            db = self.env.get_db_cnx()
            cursor = db.cursor()
            db_pass = cursor.execute("SELECT value FROM session_attribute "
                                     "WHERE sid=%s "
                                     "AND name='password'", 
                                     (user,)).fetchone()
            if db_pass and len(db_pass) > 0:
                valid = db_pass[0] == password

        return valid

    def delete_user(self, user):
        """Deletes the user account.
        Returns True if the account existed and was deleted, False otherwise.
        """
        if not self.has_user(user):
            return False

        if user == 'trac_admin':
            raise TracError('Cannot delete special user: %s' % user)

        db = self.env.get_db_cnx()
        cursor = db.cursor()
        cursor.execute("DELETE FROM session_attribute "
                       "WHERE authenticated=1 AND name='password' "
                       "AND sid=%s", (user,))
        # TODO cursor.rowcount doesn't seem to get # deleted
        # is there another way to get count instead of using has_user?
        db.commit()
        return True
