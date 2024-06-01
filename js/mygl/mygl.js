var mygl = mygl || {};


function is_scalar(s) {
  return typeof s === 'number'; // && !Number.isInteger(s);
}

function is_vec4(a) {
  if (Array.isArray(a) && a.length == 4) {
    // console.log("    - " + typeof (a[0]) + ' (' + Number.isInteger(a[0]) + ')');
    // console.log("    - " + typeof (a[1]) + ' (' + Number.isInteger(a[1]) + ')');
    // console.log("    - " + typeof (a[2]) + ' (' + Number.isInteger(a[2]) + ')');
    // console.log("    - " + typeof (a[3]) + ' (' + Number.isInteger(a[3]) + ')');
    return is_scalar(a[0]) && is_scalar(a[1]) && is_scalar(a[2]) && is_scalar(a[3]);
  }
  return false;
}
mygl.vec4 = function () {
  return [0.0, 0.0, 0.0, 0.0];
}

mygl.vec4set = function (x, y, z, w) {
  if (is_scalar(x) && is_scalar(y) && is_scalar(z) && is_scalar(w))
    return [x, y, z, w];
  return mygl.vec4();
}

mygl.vec4dot = function (u, v) {
  return u[0] * v[0] + u[0] * v[0] + u[0] * v[0] + u[0] * v[0];
}

mygl.mat4 = function () {
  return [
    [0.0, 0.0, 0.0, 0.0],
    [0.0, 0.0, 0.0, 0.0],
    [0.0, 0.0, 0.0, 0.0],
    [0.0, 0.0, 0.0, 0.0]
  ];
}

mygl.mat4ident = function () {
  return [
    [1.0, 0.0, 0.0, 0.0],
    [0.0, 1.0, 0.0, 0.0],
    [0.0, 0.0, 1.0, 0.0],
    [0.0, 0.0, 0.0, 1.0]
  ];
}

mygl.mat4scale = function (sx, sy, sz) {
  return [
    [sx, 0.0, 0.0, 0.0],
    [0.0, sy, 0.0, 0.0],
    [0.0, 0.0, sz, 0.0],
    [0.0, 0.0, 0.0, 1.0]
  ];
}

function is_mat4(a) {
  if (Array.isArray(a)) {
    if (a.length == 4) {
      // console.log("matrix format:");
      // console.log(" * " + is_vec4(a[0]));
      // console.log(" * " + is_vec4(a[1]));
      // console.log(" * " + is_vec4(a[2]));
      // console.log(" * " + is_vec4(a[3]));
      return is_vec4(a[0]) && is_vec4(a[1]) && is_vec4(a[2]) && is_vec4(a[3]);
    }
  }
  return false;
}

mygl.mat4mul = function (a, b) {
  if (is_mat4(a)) {
    if (is_mat4(b)) {
      let r = [];
      for (let i = 0; i < 4; i++) {
        r[i] = mygl.vec4();
        for (let j = 0; j < 4; j++)
          r[i][j] = a[i][0] * b[0][j] + a[i][1] * b[1][j] + a[i][2] * b[2][j] + a[i][3] * b[3][j];
      }
      return r;
    }
    else if (is_vec4(b)) {
      r = mygl.vec4();
      r[0] = mygl.vec4dot(a[0], b);
      r[1] = mygl.vec4dot(a[1], b);
      r[2] = mygl.vec4dot(a[2], b);
      r[3] = mygl.vec4dot(a[3], b);
      return r;
    }
  }
  return mygl.mat4();
}


mygl.mat4flat = function (a) {
  let r = [];
  let n = 0;
  if (is_mat4(a)) {
    for (let i = 0; i < a.length; i++)
      for (let j = 0; j < a[i].length; j++)
        r[n++] = a[j][i];
  }
  else {
    for (let i = 0; i < 16; i++)
      r[n++] = 0.0;

  }
  return r;
}