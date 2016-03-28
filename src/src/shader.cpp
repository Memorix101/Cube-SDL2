#include "cube.h"
#include "shader.h"
#include <stdio.h>

Shader::Shader(char* filename)
{
	
	m_filename = filename;
	//m_program = glCreateProgram();
	glBindProgramARB(1, m_program);
	glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, m_program);

	if (m_program == 0)
	{
		fprintf(stderr, "Can't create shader program \n");
		exit(1);
	}


	char* framentShaderText = loadShader(filename);
	glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, strlen(framentShaderText), framentShaderText);
	glEnable(GL_FRAGMENT_PROGRAM_ARB);
	//addFragmentShader(framentShaderText);
	//compileShader();
	glDisable(GL_FRAGMENT_PROGRAM_ARB);
	
}


char* Shader::loadShader(char* filename)
{
	FILE* file;

	char* path;
	strcpy(path, "..\\data\glsl\\");
	strcpy(path, filename);

	const int buffer_len = 1024;
	char buffer_string[buffer_len];

    file=fopen(path, "r");

	if (NULL == file) {
		fputs("Error while opening", stderr);
		exit(1);
	}

	memset(buffer_string, '\0', sizeof(buffer_string));

	int ret_val = fread(buffer_string, sizeof(char), buffer_len - 1, file);

	if (ret_val <= 0) {
		fputs("Error while reading", stderr);
	}

	fclose(file);

	return buffer_string;
}

void Shader::bind()
{
	glUseProgram(m_program);
}

 void Shader::addFragmentShader(char* text)
{
	addProgram(text, GL_FRAGMENT_SHADER);
}

 void Shader::updateUniforms(GLuint sampleSlot)
 {
	 unsigned int location = glGetUniformLocation(m_program, "tInput");
	 glUniform1i(location, sampleSlot);
 }

 void Shader::addProgram(char* text, int type)
{
	int shader = glCreateShader(type);
	
	if (shader == 0)
	{
		fprintf(stderr, "error creating shader %d \n", type);
		exit(1);
	}

	const GLchar* p[1];


	p[0] = text;
	
	GLint lenght[1];

	lenght[0] = sizeof(text);

	glShaderSource(shader, 1, p, lenght);
	glCompileShader(shader);
	
	GLint success;

	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		GLchar infolog[1024];

		glGetShaderInfoLog(shader, 1024, NULL, infolog);

		fprintf(stderr, "error creating shader %d : %s \n", shader, infolog);
		exit(1);
	}

	glAttachShader(m_program, shader);
}

 void Shader::compileShader() 
{
	glLinkProgram(m_program);

	GLint success = 0;
	GLchar error[1024] = { 0 };

	glGetProgramiv(m_program, GL_LINK_STATUS, &success);

	if (!success)
	{
		glGetProgramInfoLog(m_program, 1024, NULL, error);
		fprintf(stderr, "%d : %s \n", m_program, error);
		exit(1);
	}

	glValidateProgram(m_program);

	glGetProgramiv(m_program, GL_VALIDATE_STATUS, &success);

	if (!success)
	{
		glGetProgramInfoLog(m_program, 1024, NULL, error);
		fprintf(stderr, "%d : %s \n", m_program, error);
		exit(1);
	}

}


