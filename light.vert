// credit: https://github.com/xSnapi/Shadow-Casting/blob/master/Shadow%20Casting/shadow.vert

varying vec2 i_tex;

void main() {
	i_tex		= gl_TextureMatrix[0] * gl_MultiTexCoord0;
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
