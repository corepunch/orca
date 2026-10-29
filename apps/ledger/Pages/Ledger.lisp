(StackView Name="LedgerPage" Spacing=14 Padding="20 48 20 20" Overflow="hidden scroll" JustifyContent="Start" class="ledger-page"
  (TextBlock Text="ledger" FontSize=40 class="ledger-ink" IgnoreHitTest=true)
  (TextBlock Text="accounts" FontSize=18 class="ledger-accent" IgnoreHitTest=true)
  (Grid Name="AccountsLoading" Visible=(bind "DataContext/Accounts/ShowLoading") Height=64 IgnoreHitTest=true
    (ActivityIndicator class="ledger-dim"))
  (StackView Name="AccountsFailed" Visible=(bind "DataContext/Accounts/ShowError") Spacing=8 JustifyContent="Start"
    (TextBlock Text=(bind "DataContext/Accounts/ErrorMessage") FontSize=14 WordWrap=true class="ledger-error")
    (Button Name="RetryAccounts" Click="Accounts.Retry" Height=44 BorderWidth=0 class="ledger-link" "Retry"))
  (ItemsStack Name="AccountItems" Spacing=8 JustifyContent="Start" ItemNamePrefix="Account" ItemsSource=(bind "DataContext/AccountList/Items") ItemTemplate="Ledger/Templates/AccountRow")
  (StackView Name="AccountDetail" Visible=(bind "DataContext/AccountList/HasSelection") Spacing=10 JustifyContent="Start"
    (TextBlock Name="SelectedAccount" Text=(bind "DataContext/AccountList/Selected/Title") FontSize=26 class="ledger-ink" IgnoreHitTest=true)
    (TextBlock Name="SelectedBalance" Text=(bind "DataContext/AccountList/Selected/BalanceText") FontSize=16 class="ledger-dim" IgnoreHitTest=true)
    (ItemsStack Name="EntryItems" Spacing=4 JustifyContent="Start" ItemNamePrefix="Entry" ItemsSource=(bind "DataContext/Entries/Items") ItemTemplate="Ledger/Templates/EntryRow")
    (TextBlock Name="NoEntries" Visible=(bind "DataContext/Entries/ShowEmpty") Text="No entries yet." FontSize=14 class="ledger-dim" IgnoreHitTest=true)
    (Button Name="MoreEntries" Visible=(bind "DataContext/Entries/CanLoadMore") Click="Entries.LoadMore" Height=44 BorderWidth=0 TextHorizontalAlignment="Left" class="ledger-link" "more entries")
    (TextBlock Text="new entry" FontSize=18 MarginTop=8 class="ledger-accent" IgnoreHitTest=true)
    (Input Name="MemoInput" PlaceholderText="What was it?" Height=44 FontSize=15 Padding="12 0" BorderWidth=2 Text=(bind "DataContext/NewEntry/Memo" Mode=TwoWay) class="ledger-field")
    (TextBlock Name="MemoError" Text=(bind "DataContext/NewEntry/Errors/Memo") FontSize=13 class="ledger-error")
    (Input Name="AmountInput" PlaceholderText="Amount, for example -12.50" Height=44 FontSize=15 Padding="12 0" BorderWidth=2 Text=(bind "DataContext/NewEntry/Amount" Mode=TwoWay) Submit="NewEntry.Submit" class="ledger-field")
    (TextBlock Name="AmountError" Text=(bind "DataContext/NewEntry/Errors/Amount") FontSize=13 class="ledger-error")
    (Button Name="AddEntry" Click="NewEntry.Submit" IgnoreHitTest=(bind "DataContext/NewEntry/Status/IsSubmitting") Height=48 BorderWidth=0 FontWeight="Bold" class="ledger-primary" Text=(if (bind "DataContext/NewEntry/Status/IsSubmitting") "Adding…" "Add entry"))
    (TextBlock Name="FormError" Text=(bind "DataContext/NewEntry/Status/ErrorMessage") FontSize=13 WordWrap=true class="ledger-error")))
