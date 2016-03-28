class Shader
{
public:
	Shader(char* filename);
	char* loadShader(char* filename);
	void bind();
	void addProgram(char* text, int type);
	void addFragmentShader(char* text);
	void compileShader();
	void updateUniforms(unsigned int sampleSlot);

	int m_program;
	char* m_filename;
};