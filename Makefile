name = lightfetch
files = main.c
install_dir = /usr/bin/
debug:
	@echo Building debug...
	gcc -Wall -Wextra -o $(name) $(files)
	@echo Build completed! Running...
	@echo
	./$(name)
	@echo
	@exit

clean:
	@echo Building debug...
	gcc -Wall -Wextra -o $(name) $(files)
	@echo Build completed! Running...
	@echo
	./$(name)
	@echo
	rm -f $(name)
	@echo Removed output file.
	@exit

install:
	@echo Building release...
	sudo gcc -o $(install_dir)$(name) $(files)
	@echo Building and installation completed!
	@exit

uninstall:
	@echo Uninstalling lightfetch...
	sudo rm -f $(install_dir)$(name)
	rm ../lightfetch
	@echo Uninstall completed!
	@exit