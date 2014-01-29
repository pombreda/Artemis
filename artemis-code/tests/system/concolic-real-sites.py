#!/usr/bin/env python

"""
A Test Suite for running the concolic mode on some real examples.

The examples to use are taken from a CSV file passed as the first argument.
The optional second argument "dryrun" just prints the commands that would have been executed.

The CSV file has there columns: The unique name we use to identify the site, the URL which leads to the form, and the
entry-point to be used. The entry-point can either be an XPath expression or the word 'auto' to use the automatic 
entry-points which are hard-coded in ArtForm. The first line is ignored as a header.

The results from each run are saved in the following folder structure:
./Real-Site-Teating_<date>/<site name>/{<execution tree>, <overview tree>, <constraint log>}

The statistics generated by Artemis are saved to a Google Doc for logging and discussion.
"""

import sys
import os
import unittest
import csv
import time
import datetime
import subprocess
import re
import getpass
import shutil

from harness.artemis import execute_artemis
from harness.artemis import ARTEMIS_EXEC

try:
    import gdata.spreadsheet.service
except ImportError:
    print "The 'gdata' module is required to save the results."
    print "See http://code.google.com/p/gdata-python-client/ for downloads."
    sys.exit()



#SPREADSHEET_KEY = "0ApQcHUu6OpaUdDZ5TTR1UlZJYWd1U2ktM0o2YlFoX3c" # Testing
SPREADSHEET_KEY = "0ApQcHUu6OpaUdFZJZEV6LXU2VkZZa1M3QTh6TFAwUWc" # Real Log
WORKSHEET_ID = "od6"


# The unit test object. The test_* functions will be filled in by main().
class TestSequence(unittest.TestCase):
    pass


def main():
    """Reads the CSV file and runs the tests."""
    print "ArtForm concolic mode test suite"
    
    # Check the arguments
    if len(sys.argv) == 2:
        filename = sys.argv[1]
        dry_run = False
    elif len(sys.argv) == 3 and sys.argv[2].lower() == "dryrun":
        filename = sys.argv[1]
        dry_run = True
    else:
        print "Usage:"
        print "./concolic-real-sites.py tests.csv [dryrun]"
        print "The CSV file has three columns: unique site name, url, entry-point (XPath or 'auto'). The first row is a header."
        exit(1)
    
    # Read the CSV file
    sites = _read_csv_file(filename)
    
    # Create a directory for the test results from this run,
    run_name = "Test Suite Run %s" % time.strftime("%Y-%m-%d_%H-%M-%S")
    if not dry_run:
        os.mkdir(run_name)
    
    # Open the google spreadsheet used for logging
    if not dry_run:
        logger = GDataLogger(SPREADSHEET_KEY, WORKSHEET_ID)
        logger.open_spreadsheet()
    else:
        logger=None
    
    # Check the version of Artemis we are using (by git commit)
    artemis_version_url = _current_git_commit_hyperlink()
    
    # Generate a method to test each site and add it to our unit-testable class.
    date_string = time.strftime("%Y-%m-%d %H:%M:%S")
    for s in sites:
        test_name = 'test_%s' % re.sub(r'\s+', '', s[0])
        test = test_generator(s[0], s[1], s[2], dry_run=dry_run, logger=logger, version=artemis_version_url,
                              test_date=date_string, test_dir=run_name)
        setattr(TestSequence, test_name, test)
    
    # Run the unit tests
    print "Starting tests..."
    suite = unittest.TestLoader().loadTestsFromTestCase(TestSequence)
    unittest.TextTestRunner(verbosity=2).run(suite)
    



def test_generator(site_name, site_url, site_ep, dry_run=False, logger=None, version="", test_date="", test_dir="."):
    """Returns a function which will test the given website when executed."""
    
    def test(self):
        # Begin with the data we know about
        data = {}
        data['Testing Run'] = test_date
        data['Artemis Version'] = version
        data['Site'] = site_name
        data['URL'] = site_url
        data['Entry Point'] = site_ep
        
        try:
            # Clear /tmp/constraintlog so we can get the constraints from this run only.
            if not dry_run:
                open("/tmp/constraintlog", 'w').close()
            
            # Run and time the test
            start_time = time.time()
            report = execute_artemis(site_name, site_url,
                                     iterations=0,
                                     major_mode='concolic',
                                     concolic_tree_output='final-overview',
                                     verbosity='info',
                                     concolic_button=(None if site_ep.lower() == 'auto' else site_ep),
                                     dryrun=dry_run,
                                     output_parent_dir=test_dir,
                                     ignore_artemis_crash=True)
            end_time = time.time()
            data['Running Time'] = str(datetime.timedelta(seconds=(end_time - start_time)))
            data['Exit Code'] = str(report['returncode'])

            if dry_run:
                # Only print the command, exit
                return
            
            # Copy the constraint log into the current test directory.
            shutil.copyfile("/tmp/constraintlog", os.path.join(test_dir, site_name, "constraint-log.txt"))
            
            # If the return code indicates a failure, check for a core dump and create a backtrace if one exists.
            # We also delete the core dumps to save space!
            if report['returncode'] != 0 and os.path.isfile('core'):
                try:
                    bt_cmd = "gdb -q -n -ex bt -batch %s core > backtrace.txt 2>&1" % ARTEMIS_EXEC
                    subprocess.call(bt_cmd, shell=True);
                    os.remove('core')
                except OSError:
                    pass # Ignore any errors in this part.
            
            if logger is not None:
                # Add all concolic statistics from Artemis to the logged data.
                for key in report.keys():
                    if key[:10].lower() == "concolic::" and key[:29].lower() != "concolic::solver::constraint.":
                        col_header = key[10:].replace("::", "::\n")
                        data[col_header] = str(report[key])
                
                # Append the log data to the google doc.
                logger.log_data(data)
            
        except Exception as e:
            # Catch any exceptions which ocurred during the test suite itself.
            # Try to log a message that this has happened (but ignore any new errors this may cause).
            try:
                if logger is not None:
                    data['Analysis'] = "Exception of type '%s' in the test suite." % type(e).__name__
                    logger.log_data(data)
            except:
                pass
            
            # Re-raise the same exception, as if we had never caught it in the first place.
            raise e
        
        # Fail the test if the return code was non-zero.
        if 'returncode' in report and report['returncode'] != 0:
            raise Exception("Exception thrown by call to artemis (returned %d)." % report['returncode'])
    
    return test





def _read_csv_file(filename):
    """
    Reads a CSV file into a list of triples.
    If the CSV file is formatted correctly it will return a list of name/URL/EP triples
    """
    sites = []
    try:
        with open(filename, 'rb') as csvfile:
            reader = csv.reader(csvfile, skipinitialspace=True)
            reader.next() # Ignore header row
            for row in reader:
                assert(len(row) == 3) # We expect three arguments.
                sites.append(row)
        return sites
    except IOError:
        print "Could not open file", filename
        exit(1)
    except AssertionError:
        print "Encountered a row with the wrong length, aborting."
        exit(1)




# Google Spreadsheets Logging
class GDataLogger():
    """
    Allows logging to a google spreadsheet.
    New data is simply appended to the table as a new row.
    New columns are added as required."""
    
    def __init__(self, spreadsheet_key, worksheet_id):
        # If no worksheet_id is given, the GData API uses the default worksheet in the given spreadsheet instead.
        # However, this does not seem to be working, so we require it for now.
        # Also it is assumed we have one when we update the worksheet size. We could add 'od6' as the default.
        self._spreadsheet_key = spreadsheet_key
        self._worksheet_id = worksheet_id
        self._isopen = False
        self.blank_row = True # Insert a blank row before any logging is done, to visually separate tests.
    
    # TODO: This is a hack. We should use some of the proper authentication APIs instead.
    def open_spreadsheet(self):
        """Ask the user for their Google accopunt and password and open the spreadsheet."""
        
        self._sheet = gdata.spreadsheet.service.SpreadsheetsService()
        self._sheet.email = raw_input("Please enter your google account (email): ")
        self._sheet.password = getpass.getpass("Please enter your password: ")
        self._sheet.source = "ArtForm Test Logger"
        try:
            self._sheet.ProgrammaticLogin()
        except gdata.service.BadAuthentication as e:
            print "Failed to log in:", e.message
            sys.exit() # TODO: What should we actually do here?
        
        print "Login successful."
        self._isopen = True
        
        if self.blank_row:
            self.log_data({'Testing Run':' '}) # Completely blank rows are not allowed to be written

    def log_data(self, data):
        """Takes a dictionary mapping column names to values and appends it to the spreadsheet."""
        assert(self._isopen)
        
        # Make sure the columns exist
        self._ensure_columns_exist(data.keys())
        
        # Fix the column names
        data = self._prepare_new_row(data)

        # Add the row to the spreadsheet
        entry = self._sheet.InsertRow(data, self._spreadsheet_key, self._worksheet_id)
        assert(isinstance(entry, gdata.spreadsheet.SpreadsheetsList)) # Whether or not we successfully added the row.

    @staticmethod
    def _prepare_new_row(data):
        """
        Take a dict of the column headers and values to be inserted and makes the dictionary keys suitable for passing
        to the GData API.
        """
        # Column names must exist and they are given lower-case with spaces removed!
        # It seems they also have colons removed as well.
        # TODO: Is there any definitive reference for how this should be done?
        # I know that if column names have duplicate encodings they will be numbered, which is hard to handle here.
        # So I am assuming all column names will be unique, even after this transformation.
        processed_data = {}
        for column, value in data.iteritems():
            column_api_name = re.sub(r'\s+|:', '', column.lower())
            processed_data[column_api_name] = value
        
        return processed_data

    def _ensure_columns_exist(self, desired_columns):
        """
        If the given columns are not present in the spreadsheet, we add them. Input is a dict mapping column names in the
        API format to the literal values.
        """
        assert(self._isopen)
        
        # Get the existing columns
        query = gdata.spreadsheet.service.CellQuery()
        query.max_row = '1'
        query.min_row = '1'
        header_cells = self._sheet.GetCellsFeed(self._spreadsheet_key, self._worksheet_id, query=query)
        columns = [cell.content.text for cell in header_cells.entry]
        last_col_no = max([int(cell.cell.col) for cell in header_cells.entry])
        
        # Work out which we need to add
        new_columns = set(desired_columns) - set(columns)
        
        # Resize the worksheet if necessary
        # Because we use the list feed to add rows, we only need to worry about the width here.
        work_sheet = self._sheet.GetWorksheetsFeed(self._spreadsheet_key, self._worksheet_id)
        if(last_col_no + len(new_columns) > int(work_sheet.col_count.text)):
            work_sheet.col_count.text = str(last_col_no + len(new_columns))
            try:
                self._sheet.UpdateWorksheet(work_sheet)
            except gdata.service.RequestError:
                print "Error resizing worksheet."
                assert(false) # Fail this test.
        
        # Add them
        try:
            for new_col in new_columns:
                last_col_no += 1
                self._sheet.UpdateCell(1, last_col_no, new_col, self._spreadsheet_key, self._worksheet_id)
        except gdata.service.RequestError:
            print "Error adding a new column header. (maybe there is not enough space on the worksheet?)"
            assert(false) # Fail the current test



# Checking the currently checked out git commit.

def _current_git_commit():
    """Fetches the current Git commit sha, or None if it is not available."""
    try:
        commit = subprocess.check_output(["git", "rev-parse", "HEAD"]).strip()
    except CalledProcessError:
        commit = None
    return commit

def _current_git_commit_hyperlink():
    """Fetches the current Git commit and returns a link in the Google Docs format to that commit."""
    commit = _current_git_commit()
    if commit:
        link = "https://github.com/cs-au-dk/Artemis/commit/%s" % commit
        return "=hyperlink(\"%s\", \"%s\")" % (link, commit[:8])
    else:
        return ""



if __name__ == "__main__":
    main()
