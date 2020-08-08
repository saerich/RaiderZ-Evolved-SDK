using System;
using System.Collections;
using System.Collections.Generic;
using System.Collections.ObjectModel;


namespace ComLib.Diagnostics
{
    public interface IDiagnosticsService
    {
        void FilterOn(bool include, List<string> groupNames);
        void FilterOn(bool include, string groupNamesDelimited);
        void FilterOn(bool include, params DiagnosticGroup[] groups);
        IDictionary GetData();
        string GetDataTextual();
        ReadOnlyCollection<string> GroupNames { get; }
        ReadOnlyCollection<string> GroupNamesAll { get; }
        void WriteInfo(string filePath);
        void WriteInfo(string path, string referenceMessage, string commaDelimitedGroups);
        void WriteInfo(string path, string referenceMessage, params DiagnosticGroup[] groups);
    }
}
