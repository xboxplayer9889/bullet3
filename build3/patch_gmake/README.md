premake_linux64 gmake --double --standalone-examples

generates gmake folder, but it dont know anything about myProjects, so this patch can repair/modify the generated makefiles to able to compile the examplebrowser with my additinal projects.

patch.sh