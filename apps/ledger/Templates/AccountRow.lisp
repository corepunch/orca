(Button Height=64 Click="AccountList.Select" BorderWidth=0 class="ledger-row"
  (Grid Columns="1fr auto" Spacing=12 Padding="12 0" IgnoreHitTest=true
    (StackView Spacing=2 JustifyContent="Center" IgnoreHitTest=true
      (TextBlock Name="AccountTitle" Text=(bind "DataContext/Title") FontSize=20 class="ledger-ink" IgnoreHitTest=true)
      (TextBlock Text=(bind "DataContext/Kind") FontSize=13 class="ledger-dim" IgnoreHitTest=true))
    (TextBlock Name="Balance" Text=(bind "DataContext/BalanceText") FontSize=18 TextVerticalAlignment="Center" class="ledger-ink" IgnoreHitTest=true)))
