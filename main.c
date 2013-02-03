#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


char *program_name;
int verbose = 0;
char *file_path;
int count = 10;

// int block_size = -1;
unsigned long long lba = 0;
unsigned long long badblocks = 0;


struct range {
	unsigned long long from, to;
	//float average_access_time;
	// What other data will be helpful?
} **ranges;


void parse_args(int, char*[]);
void usage(void);

// void ranges_dump(void);
int add_to_long_ranges(struct range *);
void parse_data(void);
void blocks_info(void);


int main(int argc, char *argv[])
{
	parse_args(argc, argv);

	// Allocate memory for results
	// It will be a sorted array of ranges (largest first)
	struct range *new_ranges[count];
	ranges = new_ranges;

	int i;
	for (i = 0; i < count; i++)
		ranges[i] = NULL;

	// Do your job
	parse_data();
	blocks_info();

	return EXIT_SUCCESS;
}


// Get file to read and other parameters
void parse_args(int argc, char *argv[])
{
	program_name = argv[0];

	int option;
	while ((option = getopt(argc, argv, "c:v")) != -1) {
		switch (option) {
		case 'c':
			count = atoi(optarg);
			if (count <= 0) {
				fprintf(stderr, "Wrong number: %i\n", count);
				exit(EXIT_FAILURE);
			}
			break;
		case 'v':
			verbose = 1;
			break;
		default:
			usage();
		}
	}

	file_path = argv[optind];

	if (file_path == NULL)
		usage();

	if (verbose)
		fprintf(stderr, "file: %s, count: %i\n", file_path, count);
}

// Usage
void usage(void)
{
	fprintf(stderr, "Main thing this program does is to tell where are"
		" the largest areas without bad blocks\n");
	fprintf(stderr, "Data file as a first line should have"
		" `:disk_lba'"/*":block_size'"*/".\n");
	fprintf(stderr, "Other lines should be in format:"
		" `lba_address:access_time', where access time is in"
		" milliseconds (-1 means bad sector).\n");
	fprintf(stderr, "Usage: %s [-v] [-c count] file\n", program_name);
	fprintf(stderr, "\t-v tells what am I doing (no, not author)\n");
	fprintf(stderr, "\t-c number of ranges (10 is default)\n");
	exit(EXIT_FAILURE);
}


// Read records from file and put computed ranges to array
void parse_data(void)
{
	FILE *data_file;
	data_file = fopen(file_path, "r");

	if (data_file == NULL) {
		fprintf(stderr, "Cannot open file.\n");
		exit(EXIT_FAILURE);
	}

	// Get info
	if (fscanf(data_file, ":%llu"/*:%i"*/, &lba/*, &block_size*/) != 1) {
		fprintf(stderr, "Wrong file format.\n");
		exit(EXIT_FAILURE);
	}

	if (verbose)
		fprintf(stderr, "LBA: %llu"/*, Block size: %i\n"*/,
			lba/*, block_size*/);


	unsigned long long lba_address;
	int access_time;
	struct range *range_temp;

	unsigned long lines = 0;
	//int average_access_time;

	// Because disk starts at 0 and record in file could not
	range_temp = malloc(sizeof(struct range));
	range_temp->from = 0;

	// Get every line and comupte range for ones with negative access_time
	while (fscanf(data_file, "%llu:%i", &lba_address, &access_time) == 2) {
		// printf("got: %llu:%i\n", lba_address, access_time);
		if (access_time < 0) {
			badblocks++;
			range_temp->to = lba_address;
			// Add it to the array or remove if too small
			if (!add_to_long_ranges(range_temp))
				free(range_temp);
			range_temp = malloc(sizeof(struct range));
			range_temp->from = lba_address;
		}
		lines++;
	}
	// And last element
	range_temp->to = lba;/*
	range_temp->average_access_time =
		(float)average_access_time /
		(lba - range_temp->from); //*/
	if (!add_to_long_ranges(range_temp))
		free(range_temp);


	if (verbose)
		printf("Lines parsed: %lu\n", lines);

	fclose(data_file);
}
/*
void ranges_dump(void)
{
	struct range *temp;
	int i;
	for (i = 0; i < count; i++) {
		temp = ranges[i];
		if (temp == NULL)
			break;
		fprintf(stderr, " range at %llu: %llu\n",
			i, temp->to - temp->from);
	}
}//*/

// If range is large enough compute index and put it there removing last element
int add_to_long_ranges(struct range *range)
{
	unsigned long long new_range = range->to - range->from;

	if (verbose)
		fprintf(stderr, "* got range: %llu\n", new_range);

	struct range *last = ranges[count-1];

	// Too weak!
	if (last != NULL && new_range <= last->to - last->from) {
		if (verbose)
			fprintf(stderr, "range to small\n");
		return 0;
	}

	int i;
	struct range *temp;
	int my_new_place = count - 1;

	// We iterate backwards because most of the time only last elements
	// will change
	for (i = count - 1; i >= 0; i--) {
		temp = ranges[i];
		if (temp == NULL) {
			my_new_place = i;
			continue;
		}
		if (new_range > temp->to - temp->from)
			my_new_place = i;
		else
			break;
	}

	// Just in case?
	if (my_new_place == -1) {
		fprintf(stderr, "WTF?! Too small range here?\n");
		return 0;
	}

	// If there is something at the end throw it into the void!
	if (last != NULL)
		free(last);

	// Move remaining elements by one in the direction of the void
	for (i = count - 2; i >= my_new_place; i--) {
		temp = ranges[i];
		if (temp == NULL)
			continue;
		ranges[i+1] = ranges[i];
	}

	// And we have a room for our deserved element
	ranges[my_new_place] = range;

	if (verbose)
		fprintf(stderr, "added at index: %i\n", my_new_place);

	return 1;
}

void blocks_info(void)
{
	int i;
	struct range *temp;
	printf("%20s %20s %20s\n", "LBA start", "LBA end", "Blocks");
	for (i = 0; i < count; i++) {
		temp = ranges[i];
		if (temp == NULL)
			break;
			// continue;
		printf("%20llu %20llu %20llu\n",
			temp->from, temp->to, temp->to - temp->from);
		free(temp);
	}
	printf("Badblocks: %llu\n", badblocks);
}

/* previous while loop
		if (access_time > -1) {
			if (healthy) { // Still healthy!
				// Sum access times up to compute it later
				//average_access_time += access_time;
			} else { // Rising edge
//				range_temp->from = lba_address;
				healthy = 1;
				//average_access_time = 0;
			}
		} else {
			badblocks++;
			if (healthy) { // Falling egde
				range_temp->to = lba_address;//*
				range_temp->average_access_time =
					(float)average_access_time /
					(lba_address - range_temp->from)
					* block_size; ///
				healthy = 0;
				// Add it to the array or remove if too small
				if (!add_to_long_ranges(range_temp))
					free(range_temp);
				// range_temp = NULL;
				range_temp = malloc(sizeof(struct range));
				range_temp->from = lba_address;
			}
		}
		lines++;
*/
