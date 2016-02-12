import Ember from 'ember';

/**
 * This service provide support for authentication.
 *
 * It is used to maintain basic information about who
 * is currently logged in. It provides API to perform
 * authentication as well as manages authentication token
 * stored in the local storage.
 */
export default Ember.Service.extend({
  websocket: Ember.inject.service('websocket'),
  /**
   * Username of the currently logged in user.
   */
  user_id: false,

  /**
   * Are we currently attempting to authenticate.
   */
  pending: false,

  /**
   * This a defered object that is created when authentication starts.
   * It is resolved when authentication is performed.
   */
  current_auth: false,

  init()
  {
    "use strict";
    // Attempt to automatically authenticate if we can find an auth
    // token
    if (this.fetchLocalAuthToken() !== 'false')
    {
      this.authenticateWithToken(this.fetchLocalAuthToken());
    }
  },
  /**
   * Authenticate with username/password credential
   * @param username
   * @param password
   */
  authenticate(username, password, onSuccess, onFailure)
  {
    "use strict";
    var self = this;
    var ws = this.get('websocket');

    this.set('pending', true);
    this.set('current_auth', Ember.RSVP.defer());

    return ws.sendJson('create_auth_token',
      {
        username: username,
        password: password
      }).then(function (data)
    {
      self.set('pending', false);
      if (data.status === 0) // success
      {
        // Store auth token in local storage
        self.setLocalAuthToken(data.token);
        self.set('user_id', username);
        self.get('current_auth').resolve();
        if (onSuccess)
          onSuccess();
      }
      else
      {
        self.set('user_id', false);
        self.setLocalAuthToken(false);
        self.get('current_auth').reject();
        if (onFailure)
          onFailure(data.status, data.message);
      }
    });
  },
  authenticateWithToken(token)
  {
    "use strict";
    var self = this;
    var ws = this.get('websocket');

    this.set('pending', true);
    this.set('current_auth', Ember.RSVP.defer());

    return ws.sendJson('authenticate_with_token',
      {
        token: token
      }).then(function (data)
    {
      self.set('pending', false);
      if (data.status === 0)
      {
        self.set('user_id', data.user_id);
        self.get('current_auth').resolve();
      }
      else
      {
        self.get('current_auth').reject();
        console.log('Authentication token invalid');
      }
    });
  },
  /**
   * Retrieve the authentication token stored in the local storage.
   */
  fetchLocalAuthToken()
  {
    "use strict";
    if (!!localStorage.auth_token)
    {
      return localStorage.auth_token;
    }
    return false;
  },
  setLocalAuthToken(token)
  {
    "use strict";
    localStorage.auth_token = token;
  },
  /**
   * Returns whether or not a user is currently logged in.
   *
   * If an authentication attempt is in progress, we return
   * a promise instead. This promise will either be resolved
   * or rejected, depending on whether or not the authentication
   * attempt succeeded.
   * @returns {*}
   */
  isLoggedIn()
  {
    "use strict";
    if (!this.get('pending'))
      return !!this.get('user_id');
    return this.get('current_auth').promise;
  }
});